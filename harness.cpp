#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "http2.hpp"
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

std::vector<std::string> testSunnyCase() {
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
}

std::vector<std::string> testGDBUpdateSMContextFirst() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::N1_N2_TRANSFER_INITIATED);

  auto resNgap = fsm.Nsmf_PDUSession_UpdateSMContext(n11::ngap_response{});

  auto resNas = fsm.Nsmf_PDUSession_UpdateSMContext(n11::nas_response{});

  fsm.Nsmf_PDUSession_SMContextStatusNotify_Response();

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

std::vector<std::string> testN1msgNotTransferedResponse() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::N1_MSG_NOT_TRANSFERRED);

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

std::vector<std::string> testFailureCauseUnspecifiedResponse() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::FAILURE_CAUSE_UNSPECIFIED);

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

std::vector<std::string> testHTTPNotFoundErrorOnN1N2messageTransfer_Response() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      std::unexpected{http2::status::not_found});

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

std::vector<std::string> testHTTPTimeoutErrorOnN1N2messageTransfer_Response() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      std::unexpected{http2::status::timeout});

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

std::vector<std::string> testReturnFailureOnNASUpdateSMContext() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }
  auto resNas = fsm.Nsmf_PDUSession_UpdateSMContext(n11::nas_response{});

  if (resNas.has_value()) {
    problems.push_back(
        "UpdateSMContext with NAS-response doesn't return error");
  }

  return problems;
}

std::vector<std::string> testReturnFailureOnNGAPUpdateSMContext() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  auto resNgap = fsm.Nsmf_PDUSession_UpdateSMContext(n11::ngap_response{});

  if (resNgap.has_value()) {
    problems.push_back("UpdateSMContext NGAP-response doesn't return error");
  }

  return problems;
}

std::vector<std::string> testDoubleUpdateSMContext() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::N1_N2_TRANSFER_INITIATED);

  auto resNas = fsm.Nsmf_PDUSession_UpdateSMContext(n11::nas_response{});
  if (!resNas.has_value()) {
    problems.push_back(
        "First UpdateSMContext with NAS-response returned error");
  }

  resNas = fsm.Nsmf_PDUSession_UpdateSMContext(n11::nas_response{});
  if (resNas.has_value()) {
    problems.push_back(
        "Second UpdateSMContext with NAS-response didn't return error");
  }

  auto resNgap = fsm.Nsmf_PDUSession_UpdateSMContext(n11::ngap_response{});
  if (!resNgap.has_value()) {
    problems.push_back(
        "First UpdateSMContext with NGAP-response returned error");
  }

  resNgap = fsm.Nsmf_PDUSession_UpdateSMContext(n11::ngap_response{});
  if (resNgap.has_value()) {
    problems.push_back(
        "Second UpdateSMContext with NGAP-response didn't return error");
  }

  fsm.Nsmf_PDUSession_SMContextStatusNotify_Response();

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

std::vector<std::string> testTimerExpired() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::N1_N2_TRANSFER_INITIATED);

  fsm.TimerExpired();

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

std::vector<std::string> testTimerExpiredWithNAS() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::N1_N2_TRANSFER_INITIATED);

  auto resNas = fsm.Nsmf_PDUSession_UpdateSMContext(n11::nas_response{});

  fsm.TimerExpired();

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

std::vector<std::string> testTimerExpiredWithNGAP() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::N1_N2_TRANSFER_INITIATED);

  auto resGap = fsm.Nsmf_PDUSession_UpdateSMContext(n11::ngap_response{});

  fsm.TimerExpired();

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

std::vector<std::string> testTimer3592Expired() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  int before = links.log.size();

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::N1_N2_TRANSFER_INITIATED);

  fsm.Timer3592Expired(false);

  if (before == links.log.size() ||
      links.log.back() != "Namf_Communication_N1N2messageTransfer") {
    problems.push_back("N1N2messageTransfer wasn't called in Timer3592Expired");
  }

  if (fsm.isDone()) {
    problems.push_back("FSM finished working not on time");
  }

  fsm.Timer3592Expired(true);

  if (links.log.back() != "Nsmf_PDUSession_SMContextStatusNotify") {
    problems.push_back("SMContextStatus notification was not sent");
  }

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

std::vector<std::string> testUnexpectedCallsOfTimer3592Expired() {
  std::vector<std::string> problems;
  ServiceLinksMock links;

  smf::ReleaseProcedureFsm fsm{links};

  if (links.log.empty()) {
    problems.push_back("N1N2messageTransfer wasn't called");
  }

  int size = links.log.size();

  fsm.Timer3592Expired(false);

  if (links.log.size() >= 2 &&
      links.log.back() == "Namf_Communication_N1N2messageTransfer") {
    problems.push_back(
        "Unexpected call of Timer3592Expired called N1N2messageTransfer");
  }

  fsm.Timer3592Expired(true);

  if (links.log.size() >= 2 &&
      links.log.back() == "Nsmf_PDUSession_SMContextStatusNotify") {
    problems.push_back(
        "Unexpected call of Timer3592Expired called SMContextStatusNotify");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::N1_N2_TRANSFER_INITIATED);

  auto resNas = fsm.Nsmf_PDUSession_UpdateSMContext(n11::nas_response{});

  auto resNgap = fsm.Nsmf_PDUSession_UpdateSMContext(n11::ngap_response{});

  fsm.Nsmf_PDUSession_SMContextStatusNotify_Response();

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  int before = links.log.size();
  fsm.Timer3592Expired(false);
  if (links.log.size() > before &&
      links.log.back() == "Namf_Communication_N1N2messageTransfer") {
    problems.push_back("Unexpected call of Timer3592Expired after done called "
                       "N1N2messageTransfer");
  }

  before = links.log.size();
  fsm.Timer3592Expired(true);
  if (links.log.size() > before &&
      links.log.back() == "Nsmf_PDUSession_SMContextStatusNotify") {
    problems.push_back("Unexpected call of Timer3592Expired after done called "
                       "SMContextStatusNotify");
  }

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

std::vector<std::string> testDelayedN1N2messageTransferReResponse() {
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

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::N1_MSG_NOT_TRANSFERRED);
  if (fsm.isDone()) {
    problems.push_back(
        "FSM has finished work after delayed N1N2messageTransfer re-response "
        "with N1_MSG_NOT_TRANSFERRED");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::FAILURE_CAUSE_UNSPECIFIED);
  if (fsm.isDone()) {
    problems.push_back(
        "FSM has finished work after delayed N1N2messageTransfer re-response "
        "with FAILURE_CAUSE_UNSPECIFIED");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      std::unexpected{http2::status::timeout});
  if (fsm.isDone()) {
    problems.push_back(
        "FSM has finished work after delayed N1N2messageTransfer re-response "
        "with HTTP timeout error");
  }

  fsm.Namf_Communication_N1N2messageTransfer_Response(
      std::unexpected{http2::status::not_found});
  if (fsm.isDone()) {
    problems.push_back(
        "FSM has finished work after delayed N1N2messageTransfer re-response "
        "with HTTP not_found error");
  }

  fsm.Nsmf_PDUSession_SMContextStatusNotify_Response();

  int before = links.log.size();
  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::N1_MSG_NOT_TRANSFERRED);
  if (before < links.log.size()) {
    problems.push_back(
        "FSM has finished work after delayed N1N2messageTransfer re-response "
        "with N1_MSG_NOT_TRANSFERRED in done state");
  }

  before = links.log.size();
  fsm.Namf_Communication_N1N2messageTransfer_Response(
      n11::n1n2_transfer_cause::FAILURE_CAUSE_UNSPECIFIED);
  if (before < links.log.size()) {
    problems.push_back(
        "FSM has finished work after delayed N1N2messageTransfer re-response "
        "with FAILURE_CAUSE_UNSPECIFIED in done state");
  }

  if (!fsm.isDone()) {
    problems.push_back("FSM hasn't finished working");
  }

  return problems;
}

int main() {
  return runAll(
      {{"Sunny case", testSunnyCase},
       {"GDB-UpdateSMContext first", testGDBUpdateSMContextFirst},
       {"N1N2messageTransfer response with N1_MSG_NOT_TRANSFERRED",
        testN1msgNotTransferedResponse},
       {"N1N2messageTransfer response with FAILURE_CAUSE_UNSPECIFIED",
        testFailureCauseUnspecifiedResponse},
       {"N1N2messageTransfer response Not Found HTTP Error",
        testHTTPNotFoundErrorOnN1N2messageTransfer_Response},
       {"N1N2messageTransfer response Timeout HTTP Error",
        testHTTPTimeoutErrorOnN1N2messageTransfer_Response},
       {"Not expected UpdateSMContext with NAS-response",
        testReturnFailureOnNASUpdateSMContext},
       {"Not expected UpdateSMContext with NGAP-response",
        testReturnFailureOnNGAPUpdateSMContext},
       {"Double UpdateSMContext", testDoubleUpdateSMContext},
       {"Test TimerExpired function", testTimerExpired},
       {"Test TimerExpired function after UpdateContext with NAS-response",
        testTimerExpiredWithNAS},
       {"Test TimerExpired function after UpdateContext with NGAP-response",
        testTimerExpiredWithNGAP},
       {"Test Timer3592Expired", testTimer3592Expired},
       {"Test Unexpected calls of Timer3592Expired",
        testUnexpectedCallsOfTimer3592Expired},
       {"Test Delayed N1N2messageTransfer Re-Response",
        testDelayedN1N2messageTransferReResponse}});
}