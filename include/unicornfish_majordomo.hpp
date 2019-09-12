// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/Damselfish/GLUT Extensions
// Copyright (C) 2017 Jonathan Metzgar
// All rights reserved.
//
// This program is free software : you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.If not, see <https://www.gnu.org/licenses/>.
//
// For any other type of licensing, please contact me at jmetzgar@outlook.com
#ifndef UNICORNFISH_MAJORDOMO_HPP
#define UNICORNFISH_MAJORDOMO_HPP

namespace Uf
{
	namespace Majordomo
	{
		enum class Command
		{
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

		extern const char *WorkerId;
		extern const char *BrokerId;
		extern const char *ClientId;
	} // namespace Majordomo
} // namespace Uf

#endif