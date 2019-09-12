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
#ifndef UNICORNFISH_CURL_HPP
#define UNICORNFISH_CURL_HPP

#include <string>
#include <future>
#include <unicornfish.hpp>

namespace Uf
{

class Curl
{
  public:
	using StringTimePair = std::pair<std::string, double>;
	using StringTimePairFuture = std::future<std::pair<std::string, double>>;

	Curl();
	~Curl();

	std::string Get(const std::string &url);
	StringTimePairFuture AsyncGet(const std::string &url);

  private:
	CURL *curl;
	std::string get_data;
	std::string post_data;

	static size_t write_callback(char *buffer, size_t size, size_t nitems, void *instream);
};
} // namespace Uf

#endif
