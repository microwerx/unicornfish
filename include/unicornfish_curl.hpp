#ifndef UNICORNFISH_CURL_HPP
#define UNICORNFISH_CURL_HPP

#include <unicornfish_base.hpp>

namespace Uf {
	class Curl {
	public:
		using StringTimePair = std::pair<std::string, double>;
		using StringTimePairFuture = std::future<std::pair<std::string, double>>;

		Curl();
		~Curl();

		std::string Get(const std::string& url);
		StringTimePairFuture AsyncGet(const std::string& url);

	private:
		CURL* curl;
		std::string get_data;
		std::string post_data;

		static size_t write_callback(char* buffer, size_t size, size_t nitems, void* instream);
	};
} // namespace Uf

#endif
