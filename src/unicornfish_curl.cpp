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

#include <unicornfish_curl.hpp>

namespace Uf
{
	Curl::Curl()
	{
	}

	Curl::~Curl()
	{
	}

	std::string Curl::Get(const std::string &url)
	{
		std::string result;
		curl = curl_easy_init();
		if (curl) {
			CURLcode res;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Curl::write_callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);
			res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
			}
			curl_easy_cleanup(curl);
		}
		return result;
	}

	Curl::StringTimePairFuture Curl::AsyncGet(const std::string &url)
	{
		StringTimePairFuture future_result = std::async(std::launch::async, [url]() {
			StringTimePair result;
			result.second = Hf::Log.getSecondsElapsed();
			CURL *curl = curl_easy_init();
			if (curl) {
				CURLcode res;
				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Curl::write_callback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);
				res = curl_easy_perform(curl);
				if (res != CURLE_OK) {
					result.first = "<Curl::AsyncGet ERROR [" + std::string(curl_easy_strerror(res)) + "]> " + url;
				}
				curl_easy_cleanup(curl);
			}
			result.second = Hf::Log.getSecondsElapsed() - result.second;
			return result;
		});
		return future_result;
	}

	size_t Curl::write_callback(char *buffer, size_t size, size_t nitems, void *instream)
	{
		if (!instream)
			return 0;
		StringTimePair *data = (Curl::StringTimePair *)instream;

		data->first.append(buffer, size);

		return size;
	}
} // namespace Uf