#include "release_procedure_fsm.hpp"

namespace smf {

ReleaseProcedureFsm::ReleaseProcedureFsm(IServiceLinks &links)
    : m_links{links} {

  m_links.Namf_Communication_N1N2messageTransfer(
      n11::n1_nas_payload{.command = n11::nas_command::PDU_SESSION_RELEASE,
                          .cause = n11::nas_cause::REGULAR_DEACTIVATION},
      n11::n2_ngap_payload{.command = n11::ngap_command::RESOURCE_RELEASE});

  m_state = state::n1n2_transfer_sent;
}

void ReleaseProcedureFsm::Namf_Communication_N1N2messageTransfer_Response(
    std::expected<n11::n1n2_transfer_cause, http2::status> cause) {
  if (m_state == state::done || m_state == state::notification_sent) {
    return;
  }

  if (!cause.has_value()) {
    m_links.StopTimer3592();
    m_links.StopTimer();
    m_state = state::done;
    return;
  }

  switch (cause.value()) {
    using enum n11::n1n2_transfer_cause;
  case N1_N2_TRANSFER_INITIATED:
    if (m_state == state::n1n2_transfer_sent) {
      m_state = state::n1n2_transfer_ok_response_received;
      m_links.StartTimer();
      m_links.StartTimer3592();
    }
    break;
  case N1_MSG_NOT_TRANSFERRED:
  case FAILURE_CAUSE_UNSPECIFIED:
    m_links.StopTimer3592();
    m_links.StopTimer();
    m_links.Nsmf_PDUSession_SMContextStatusNotify(
        n11::sm_context_status::RELEASED);
    m_state = state::done;
    break;
  }
}

std::expected<void, n11::update_sm_context_error_cause>
ReleaseProcedureFsm::Nsmf_PDUSession_UpdateSMContext(n11::nas_response) {
  switch (m_state) {
  case state::n1n2_transfer_ok_response_received: {
    m_state = state::nas_ok_received;
    m_links.StopTimer3592();
    return {};
  }
  case state::ngap_ok_received: {
    m_links.StopTimer3592();
    m_links.StopTimer();
    m_links.Nsmf_PDUSession_SMContextStatusNotify(
        n11::sm_context_status::RELEASED);
    m_state = state::notification_sent;
    return {};
  }
  default: {
    return std::unexpected{
        n11::update_sm_context_error_cause::UNSPECIFIED_NF_FAILURE};
  }
  }
}

std::expected<void, n11::update_sm_context_error_cause>
ReleaseProcedureFsm::Nsmf_PDUSession_UpdateSMContext(n11::ngap_response) {
  switch (m_state) {
  case state::n1n2_transfer_ok_response_received: {
    m_state = state::ngap_ok_received;
    return {};
  }
  case state::nas_ok_received: {
    m_links.StopTimer();
    m_links.Nsmf_PDUSession_SMContextStatusNotify(
        n11::sm_context_status::RELEASED);
    m_state = state::notification_sent;
    return {};
  }
  default: {
    return std::unexpected{
        n11::update_sm_context_error_cause::UNSPECIFIED_NF_FAILURE};
  }
  }
}

void ReleaseProcedureFsm::Nsmf_PDUSession_SMContextStatusNotify_Response() {
  if (m_state != state::notification_sent) {
    return;
  }
  m_state = state::done;
}

void ReleaseProcedureFsm::TimerExpired() {
  if (m_state == state::n1n2_transfer_ok_response_received ||
      m_state == state::nas_ok_received || m_state == state::ngap_ok_received) {
    m_links.StopTimer3592();
    m_links.StopTimer();
    m_links.Nsmf_PDUSession_SMContextStatusNotify(
        n11::sm_context_status::RELEASED);
    m_state = state::done;
  }
}

void ReleaseProcedureFsm::Timer3592Expired(bool last) {
  if (m_state != state::ngap_ok_received &&
      m_state != state::n1n2_transfer_ok_response_received) {
    return;
  }

  if (last) {
    m_links.StopTimer();
    m_links.Nsmf_PDUSession_SMContextStatusNotify(
        n11::sm_context_status::RELEASED);
    m_state = state::done;
    return;
  }

  m_links.Namf_Communication_N1N2messageTransfer(
      n11::n1_nas_payload{.command = n11::nas_command::PDU_SESSION_RELEASE,
                          .cause = n11::nas_cause::REGULAR_DEACTIVATION},
      std::nullopt);
}

} // namespace smf