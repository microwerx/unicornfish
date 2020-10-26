#ifndef UNICORNFISH_CLIENT_HPP
#define UNICORNFISH_CLIENT_HPP

#include <unicornfish_base.hpp>
#include <unicornfish_socket.hpp>
#include <unicornfish_message.hpp>

namespace Uf {
	class Client {
	public:
		Client();
		Client(const std::string& endpoint);
		Client(const char* endpoint);
		~Client();

		operator bool() const { return clientSocket; }

		bool ConnectToBroker(const std::string& endpoint);
		bool ConnectToBroker(const char* endpoint);
		void Disconnect();
		void ClearReply();

		bool SendRequest(const std::string& service, const Message& msg);
		bool SendRequest(const char* service, const Message& msg);
		bool PollReply();
		bool WaitReply();

		bool SendWaitReply(const std::string& service, const Message& request);
		bool SendWaitReply(const char* service, const Message& request);
		Message& GetReply() { return replyMessage; }
		const std::string& GetReplyService() { return replyService; }
		const std::string& GetReplyHeader() { return replyHeader; }

	private:
		std::string brokerEndpoint;
		Socket clientSocket;
		Message replyMessage;
		std::string replyHeader;
		std::string replyService;
		const int requestTimeoutMsec = 2500;
		const int requestRetries = 3;
		bool verbose = false;

		bool SendRequestToBroker(const std::string& service, const Message& msg);
		bool SendRequestToBroker(const char* service, const Message& msg);
	};
} // namespace Uf

#endif
