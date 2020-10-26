#include "unicornfish_pch.hpp"
#include <unicornfish_curl.hpp>
#include <hatchetfish.hpp>

namespace Uf {
	Curl::Curl() {}

	Curl::~Curl() {}

	std::string Curl::Get(const std::string& url) {
		std::string result;
		curl = curl_easy_init();
		if (curl) {
			CURLcode res;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Curl::write_callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&result);
			res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
			}
			curl_easy_cleanup(curl);
		}
		return result;
	}

	Curl::StringTimePairFuture Curl::AsyncGet(const std::string& url) {
		StringTimePairFuture future_result = std::async(std::launch::async, [url]() {
			StringTimePair result;
			result.second = HFLOG_SECS_ELAPSED();
			CURL* curl = curl_easy_init();
			if (curl) {
				CURLcode res;
				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Curl::write_callback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&result);
				res = curl_easy_perform(curl);
				if (res != CURLE_OK) {
					result.first = "<Curl::AsyncGet ERROR [" + std::string(curl_easy_strerror(res)) + "]> " + url;
				}
				curl_easy_cleanup(curl);
			}
			result.second = HFLOG_SECS_ELAPSED() - result.second;
			return result;
														});
		return future_result;
	}

	size_t Curl::write_callback(char* buffer, size_t size, size_t nitems, void* instream) {
		if (!instream || nitems == 0)
			return 0;
		StringTimePair* data = (Curl::StringTimePair*)instream;

		data->first.append(buffer, size);

		return size;
	}
} // namespace Uf
