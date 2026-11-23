#pragma once

namespace n11 {

enum class nas_cause { REGULAR_DEACTIVATION };
enum class nas_command { PDU_SESSION_RELEASE };

struct n1_nas_payload {
  nas_command command;
  nas_cause cause;
};

enum class ngap_command { RESOURCE_RELEASE };
struct n2_ngap_payload {
  ngap_command command;
};

enum class n1n2_transfer_cause {
  N1_N2_TRANSFER_INITIATED,
  N1_MSG_NOT_TRANSFERRED,
  FAILURE_CAUSE_UNSPECIFIED
};

enum class ngap_response { RESOURCE_RELEASE_RESPONSE };
enum class nas_response { PDU_SESSION_RELEASE_COMPLETE };

enum class update_sm_context_error_cause { UNSPECIFIED_NF_FAILURE };

enum class sm_context_status { RELEASED };

} // namespace n11
