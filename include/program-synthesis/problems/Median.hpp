#pragma once

#include <utility>

#include "BaseProblem.hpp"
#include "psb/readers/Median.hpp"
#include "../TestResult.hpp"

namespace psynth::problems {

struct Median : public BaseProblem {
  using reader_t = psb::readers::Median;
  using input_t = typename reader_t::input_t;
  using output_t = typename reader_t::output_t;
  using test_case_t = std::pair<input_t, output_t>;
  using prob_hw_t = NumericOutputHardware;

  size_t input_sig_event_id = 0;

  template<typename HARDWARE_T>
  void ConfigureHardware(HARDWARE_T& hw) {
    auto& hw_component = hw.GetCustomComponent();
    hw_component.template CreateProblemHardware<prob_hw_t>();
  }

  template<typename INST_LIB_T>
  void AddInstructions(INST_LIB_T& inst_lib) {
    using hardware_t = typename INST_LIB_T::hardware_t;
    using inst_t = typename hardware_t::inst_t;

    inst_lib.AddInst(
      "SubmitOutput",
      [](hardware_t& hw, const inst_t& inst) {
        auto& component = hw.GetCustomComponent().template GetProbHW<prob_hw_t>();
        auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
        auto& mem_state = call_state.GetMemory();
        double output = mem_state.AccessWorking(inst.GetArg(0));
        component.SubmitOutput(output);
      },
      "Submit output"
    );

  }

  template<typename EVENT_LIB_T>
  void AddEvents(EVENT_LIB_T& event_lib) {
    input_sig_event_id = event_lib.GetID("NumericInputSignal");
  }

  template<typename HARDWARE_T, typename ORG_T>
  void InitTest(HARDWARE_T& hw, ORG_T& org, const test_case_t& test_io) {
    // Load test inputs into hardware
    const input_t& input = test_io.first;
    const size_t tag_w = HARDWARE_T::tag_t::GetSize();
    hw.QueueEvent(
      NumericMessageEvent<tag_w>{
        input_sig_event_id,
        hw.GetCustomComponent().GetInputTag(),
        {
          {0, (double)input[0]},
          {1, (double)input[1]},
          {2, (double)input[2]}
        }
      }
    );
  }

  template<typename HARDWARE_T, typename ORG_T>
  TestResult EvaluateOutput(HARDWARE_T& hw, ORG_T& org, const test_case_t& test_io) {
    // Get correct output
    const int correct_output = test_io.second;
    // Get reference to problem component
    auto& prob_component = hw.GetCustomComponent().template GetProbHW<prob_hw_t>();
    // Did the program record any output?
    const bool has_output = prob_component.HasOutput();
    // Is the output correct?
    const bool correct = has_output && (prob_component.GetOutput() == correct_output);
    const double partial_credit = (correct) ? 1.0 : 0.0;
    return {has_output, correct, partial_credit};
  }

};

}