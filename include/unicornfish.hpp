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
#ifndef UNICORNFISH_HPP
#define UNICORNFISH_HPP

#ifdef _WIN32
#pragma comment(lib, "libsodium.lib")
#pragma comment(lib, "libzmq.lib")
#pragma comment(lib, "libczmq.lib")
#pragma comment(lib, "libcurl_a.lib")
#pragma comment(lib, "fluxions.lib")
#pragma comment(lib, "fluxions-gte.lib")
#pragma comment(lib, "fluxions-deps.lib")
#pragma comment(lib, "hatchetfish.lib")
#endif

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <inttypes.h>
#include <zmq.h>
#include <czmq.h>

//#ifdef _WIN32
//#define CURL_STATICLIB
//#endif
//#include <sodium.h>
//#include <curl/curl.h>

namespace Uf
{
	bool Init();
	bool Kill();
}

#endif
