#include "unicornfish_pch.hpp"
#include <unicornfish_base.hpp>
#include <unicornfish_curl.hpp>

namespace Uf {
	bool Init() {
		curl_global_init(CURL_GLOBAL_ALL);

		return true;
	}

	bool Kill() {
		curl_global_cleanup();

		return true;
	}
} // namespace Uf
