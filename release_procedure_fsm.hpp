#pragma once

#include <expected>

#include "http2.hpp"
#include "n11.hpp"
#include "service_links.hpp"

namespace smf {

class ReleaseProcedureFsm {
public:
  ReleaseProcedureFsm(IServiceLinks &links);

  // Входящие запросы
  std::expected<void, n11::update_sm_context_error_cause>
      Nsmf_PDUSession_UpdateSMContext(n11::ngap_response);

  std::expected<void, n11::update_sm_context_error_cause>
      Nsmf_PDUSession_UpdateSMContext(n11::nas_response);

  // Ответы на запросы, сделанные через IServiceLinks
  void Namf_Communication_N1N2messageTransfer_Response(
      std::expected<n11::n1n2_transfer_cause, http2::status> cause);

  void Nsmf_PDUSession_SMContextStatusNotify_Response();

  // Таймеры
  void TimerExpired();
  void Timer3592Expired(bool last);

  // Проверка завершения работы
  bool isDone() const { return m_state == state::done; }

private:
  enum class state {
    initial,
    n1n2_transfer_sent,
    n1n2_transfer_ok_response_received,
    nas_ok_received,
    ngap_ok_received,
    notification_sent,
    done
  } m_state{state::initial};

  IServiceLinks &m_links; // Исходящие запросы и установка таймера
};

} // namespace smf