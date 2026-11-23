#pragma once

#include <optional>
#include <string>

#include "n11.hpp"

namespace smf {

class IServiceLinks {
public:
  virtual ~IServiceLinks() = default;
  virtual void Namf_Communication_N1N2messageTransfer(
      n11::n1_nas_payload, std::optional<n11::n2_ngap_payload>) = 0;
  virtual void Nsmf_PDUSession_SMContextStatusNotify(
      n11::sm_context_status) = 0; // обращается к AMF, используя механизм
                                   // колбэков
  virtual void StartTimer() = 0;

  virtual void StopTimer() = 0;

  virtual void StartTimer3592() = 0;

  virtual void StopTimer3592() = 0;
};

} // namespace smf