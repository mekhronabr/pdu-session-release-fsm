#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "n11.hpp"
#include "release_procedure_fsm.hpp"

namespace {

class ServiceLinksMock final : public smf::IServiceLinks {
public:
  std::vector<std::string> log;

  void Namf_Communication_N1N2messageTransfer(
      n11::n1_nas_payload, std::optional<n11::n2_ngap_payload>) override {
    log.push_back("Namf_Communication_N1N2messageTransfer");
  }

  void Nsmf_PDUSession_SMContextStatusNotify(n11::sm_context_status) override {
    log.push_back("Nsmf_PDUSession_SMContextStatusNotify");
  }

  void StartTimer() override { log.push_back("StartTimer"); }

  void StopTimer() override {}

  void StartTimer3592() override { log.push_back("StartTimer3592"); }

  void StopTimer3592() override {}
};

struct Case {
  std::string name;
  std::function<std::vector<std::string>()> run; // empty == pass
};

int runAll(std::vector<Case> const &cases) {
  std::size_t passed = 0;
  for (auto const &c : cases) {
    auto problems = c.run();
    if (problems.empty()) {
      ++passed;
      std::cout << std::format("[PASS] {}\n", c.name);
    } else {
      std::cout << std::format("[FAIL] {}\n", c.name);
      for (auto const &p : problems) {
        std::cout << std::format("       - {}\n", p);
      }
    }
  }

  std::cout << std::format("\n{}/{} cases passed\n", passed, cases.size());
  return passed == cases.size() ? 0 : 1;
}

} // namespace

int main() {

  auto testSunnyCase = [] {
    std::vector<std::string> problems;
    ServiceLinksMock links;
    
    smf::ReleaseProcedureFsm fsm{links};

    if (links.log.empty()) {
      problems.push_back("N1N2messageTransfer wasn't called");
    }

    fsm.Namf_Communication_N1N2messageTransfer_Response(
        n11::n1n2_transfer_cause::N1_N2_TRANSFER_INITIATED);

    auto resNas = fsm.Nsmf_PDUSession_UpdateSMContext(n11::nas_response{});

    auto resNgap = fsm.Nsmf_PDUSession_UpdateSMContext(n11::ngap_response{});

    fsm.Nsmf_PDUSession_SMContextStatusNotify_Response();

    if (!fsm.isDone()) {
      problems.push_back("FSM hasn't finished working");
    }

    return problems;
  };

  return runAll({
      {"Sunny case", testSunnyCase},
  });
}