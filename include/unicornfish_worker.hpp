#ifndef UNICORNFISH_WORKER_HPP
#define UNICORNFISH_WORKER_HPP

#include <unicornfish_base.hpp>
#include <unicornfish_socket.hpp>
#include <unicornfish_message.hpp>

namespace Uf {
	class Worker {
	public:
		Worker();
		Worker(const char* endpoint, const char* service);
		virtual ~Worker();

		operator bool() const { return workerSocket; }

		void Run();

		bool ConnectToBroker(const char* endpoint, const char* service);
		void Disconnect();
		bool SendReplyWaitRequest(const Message& reply);
		bool SendReply(const Message& reply);
		bool WaitRequest();
		const std::string& GetServiceName() const { return serviceName; }
		const std::string& GetBrokerEndpoint() const { return brokerEndpoint; }
		// const std::string &GetRequestHeader() const { return requestHeader; }
		// const std::string &GetRequestCommand() const { return requestCommand; }
		Message& GetRequest() { return requestMessage; }
		const Message& GetRequest() const { return requestMessage; }

		/// Implementers will want to implement this method to process the incoming message
		virtual void OnProcessRequest(const Message& request, Message& reply);
	private:
		Message requestMessage;
		Message replyMessage;
		Frame replyTo;
		std::string replyToHexAddress;

		std::string brokerEndpoint;
		std::string serviceName;
		Socket workerSocket;

		int64_t heartbeatTime;
		size_t liveness = 3;
		size_t retries = 3;
		bool expectReply = false;
		bool verbose = false;

		bool ConnectToBroker();
		bool SendCommandToBroker(Majordomo::Command command);
		bool SendCommandToBroker(Majordomo::Command command, Message& msg);
		bool SendCommandToClient(Majordomo::Command command, Message& msg);
	};
} // namespace Uf
#endif
