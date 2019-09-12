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
}
}