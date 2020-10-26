#ifndef UNICORNFISH_BROKER_HPP
#define UNICORNFISH_BROKER_HPP

#include <unicornfish_base.hpp>
#include <unicornfish_socket.hpp>
#include <unicornfish_message.hpp>

namespace Uf {
	class Socket;
	class BrokerWorker;
	class BrokerServiceInfo;

	class BrokerWorker {
	public:
		std::string name;
		std::string key;
		std::string serviceName;
		Frame* identityFrame;
		int64_t expiry = 0;
		BrokerServiceInfo* service = nullptr;
		size_t count = 0;
		bool heartbeatSent = false;

		BrokerWorker() {
			identityFrame = new Frame();
		}

		~BrokerWorker() {
			delete identityFrame;
		}

		operator bool() const { return (bool)identityFrame; }

		void RenewExpiry(int64_t new_expiry) {
			expiry = new_expiry;
		}

		void SendDisconnect(Socket& socket) {
			Message msg;
			msg.Create();
			msg.Push(Majordomo::Command::Disconnect);
			msg.Push(Majordomo::WorkerId);
			msg.Push(""); // Wrap(identityFrame);
			msg.Send(socket);
		}

		void SendHeartbeat(Socket& socket) {
			Message msg;
			msg.Create();
			msg.Push(Majordomo::Command::Heartbeat);
			msg.Push(Majordomo::WorkerId);
			msg.Push(""); // msg.Wrap(identityFrame);
			msg.Send(socket);
			heartbeatSent = true;
		}

		void SendCommand(Majordomo::Command command, Message& msg, Socket& socket) {
			std::string workerHexAddress = identityFrame->GetHexData();
			if (!msg)
				msg.Create();
			msg.Push(command);
			msg.Push(Majordomo::WorkerId);
			msg.Wrap(*identityFrame);
			// if (verbose) printf("broker::service sending client request to worker %s\n", workerHexAddress.c_str());
			msg.Send(socket);
		}

		void Wait() {
			expiry = zclock_time() + Majordomo::DefaultHeartbeatIntervalMS;
		}
	};

	class BrokerServiceInfo {
	public:
		std::string name;
		std::list<Message> requests;
		std::list<BrokerWorker*> waitingWorkers;
		size_t numWorkers = 0;

		void AddWorker(BrokerWorker* worker) {
			if (!worker)
				return;
			waitingWorkers.push_back(worker);
			numWorkers++;
		}

		void RemoveWorker(BrokerWorker* worker) {
			if (!worker)
				return;
			waitingWorkers.remove(worker);
			numWorkers--;
		}

		// send requests to waiting workers
		void Dispatch(Socket& socket) {
			while (!waitingWorkers.empty() && !requests.empty()) {
				BrokerWorker* worker = PopWorker();
				worker->SendCommand(Majordomo::Command::Request, requests.front(), socket);
				requests.pop_front();
			}
		}

		BrokerWorker* PopWorker() {
			if (waitingWorkers.empty())
				return nullptr;
			BrokerWorker* worker = waitingWorkers.front();
			waitingWorkers.pop_front();
			return worker;
		}
	};

	class Broker {
	public:
		Broker();
		Broker(const char* endpoint);
		~Broker();

		operator bool() const { return (bool)brokerSocket; }

		bool Create(const char* endpoint);
		void Delete();
		bool ProcessWorkerMessage(Frame& sender, Message& msg);
		bool ProcessClientMessage(Frame& sender, Message& msg);
		void Purge();

		void Run();
		bool RunLoop();

		int const GetNumWorkers() const { return (int)workers.size(); }
		int const GetNumWaitingWorkers() const { return (int)waitingWorkers.size(); }
		int const GetNumRequests() const {
			int count = 0;
			for (auto& service : services)
				count += (int)service.second.requests.size();
			return count;
		}

	private:
		bool WorkerExists(const std::string& workerName) const;
		BrokerWorker* FindWorker(const std::string& workerName);
		void DisconnectWorker(BrokerWorker* worker);
		void DeleteWorker(const std::string& workerName);
		void WaitWorker(BrokerWorker* worker);

		bool ServiceExists(const std::string& serviceName) const;
		BrokerServiceInfo* FindService(const std::string& serviceName);
		void DispatchService(BrokerServiceInfo* service, Message& msg);
		void DispatchService(BrokerServiceInfo* service);

		std::string endpoint;
		Socket brokerSocket;
		std::map<std::string, BrokerServiceInfo> services;
		std::map<std::string, BrokerWorker> workers;
		std::list<BrokerWorker*> waitingWorkers;
		int64_t heartbeatTime = 0;
		bool verbose = false;
	}; // class Broker
} // namespace Uf

#endif