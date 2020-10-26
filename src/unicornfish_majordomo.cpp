#include "unicornfish_pch.hpp"
#include <czmq.h>
#include <unicornfish_majordomo.hpp>

namespace Uf {
	namespace Majordomo {
		const int DefaultTimeoutMS = 2500;
		const int DefaultRetries = 3;
		const int DefaultHeartbeatDelayMS = 2500;
		const int DefaultReconnectDelayMS = 2500;
		const int DefaultHearbeatAliveness = 3;
		const int DefaultHeartbeatIntervalMS = 2500;
		const int DefaultHeartbeatIntervalPollMS = DefaultHeartbeatIntervalMS * ZMQ_POLL_MSEC;

		const char* WorkerId = "MDPW01";
		const char* BrokerId = "MDPB01";
		const char* ClientId = "MDPC01";
	} // namespace Majordomo
} // namespace Uf
