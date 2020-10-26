#ifndef UNICORNFISH_MAJORDOMO_HPP
#define UNICORNFISH_MAJORDOMO_HPP

namespace Uf {
	namespace Majordomo {
		enum class Command {
			None = 0,
			Ready = 1,
			Request = 2,
			Reply = 3,
			Heartbeat = 4,
			Disconnect = 5
		};

		extern const int DefaultTimeoutMS;
		extern const int DefaultRetries;
		extern const int DefaultHeartbeatDelayMS;
		extern const int DefaultHeartbeatExpiryMS; // Expiry means "the end of the period for which something is valid"
		extern const int DefaultReconnectDelayMS;
		extern const int DefaultHearbeatAliveness;
		extern const int DefaultHeartbeatIntervalMS;
		extern const int DefaultHeartbeatIntervalPollMS;

		extern const char* WorkerId;
		extern const char* BrokerId;
		extern const char* ClientId;
	} // namespace Majordomo
} // namespace Uf

#endif