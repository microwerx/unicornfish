#include "unicornfish_pch.hpp"
#include <unicornfish_broker.hpp>
#include <unicornfish_majordomo.hpp>

namespace Uf {
	Broker::Broker() {}

	Broker::Broker(const char* endpoint) {
		Create(endpoint);
	}

	Broker::~Broker() {
		Delete();
	}

	bool Broker::Create(const char* endpoint_) {
		Delete();
		if (!brokerSocket.NewRouter(endpoint_)) {
			return false;
		}
		endpoint = endpoint_;
		heartbeatTime = zclock_time() + Majordomo::DefaultHeartbeatIntervalMS;
		return true;
	}

	void Broker::Delete() {
		if (brokerSocket) {
			brokerSocket.Delete();
			services.clear();
			workers.clear();
			waitingWorkers.clear();
		}
	}

	bool Broker::ProcessWorkerMessage(Frame& sender, Message& msg) {
		std::string workerName = sender.GetHexData();
		bool workerIsReady = WorkerExists(workerName);
		BrokerWorker* worker = FindWorker(workerName);
		worker->identityFrame->Copy(sender);

		Majordomo::Command command = msg.PopCommand();
		if (command == Majordomo::Command::Ready) {
			if (workerIsReady) {
				if (verbose)
					printf("broker: received READY from worker, but worker was ready, disconnecting worker\n");
				DisconnectWorker(worker);
			}
			else if (sender.strncmp("mmi.", 4)) {
				// workers aren't allowed to have services that start with "mmm."
				if (verbose)
					printf("broker: received MMI from worker, but worker can't have MMIs, disconnecting worker\n");
				DisconnectWorker(worker);
			}
			else {
				if (verbose)
					printf("broker: received READY from worker, adding worker to waiting list\n");
				Frame serviceFrame = msg.PopFrame();
				worker->service = FindService(serviceFrame.GetStrData());
				worker->service->numWorkers++;
				WaitWorker(worker);
			}
		}
		else if (command == Majordomo::Command::Reply) {
			if (workerIsReady) {
				// Frame 0 is Empty
				// Frame 1 is MDPW01
				// Frame 2 is 0x03 (REPLY)
				// Frame 3 is Client address
				// Frame 4 is Empty
				// Frame 5+ is Request body
				Frame clientReturnEnvelope = msg.Unwrap();
				std::string clientHexAddress = clientReturnEnvelope.GetHexData();
				msg.Push(worker->service->name);
				msg.Push(Majordomo::ClientId);
				msg.Push(clientReturnEnvelope);
				if (verbose)
					printf("broker: received reply from worker, now sending reply to client %s\n", clientHexAddress.c_str());
				msg.Send(brokerSocket);
				WaitWorker(worker);
			}
			else {
				if (verbose)
					printf("broker: received reply from worker, but worker was not ready, disconnecting worker\n");
				DisconnectWorker(worker);
			}
		}
		else if (command == Majordomo::Command::Heartbeat) {
			if (workerIsReady) {
				if (verbose)
					printf("broker: received heartbeat from worker, renewing expiry\n");
				worker->RenewExpiry(zclock_time() + Majordomo::DefaultHeartbeatIntervalMS);
			}
			else {
				if (verbose)
					printf("broker: received heartbeat from worker, but worker was not ready, disconnecting worker\n");
				DisconnectWorker(worker);
			}
		}
		else if (command == Majordomo::Command::Disconnect) {
			if (verbose)
				printf("broker: received disconnect from worker, deleting worker\n");
			DeleteWorker(workerName);
		}
		else {
			// invalid input message
			if (verbose)
				printf("broker: received invalid input message\n");
		}
		return true;
	}

	bool Broker::ProcessClientMessage(Frame& sender, Message& msg) {
		if (msg.Size() < 2)
			return false;

		Frame serviceFrame = msg.PopFrame();
		std::string requestedServiceName = serviceFrame.GetStrData();
		BrokerServiceInfo* service = FindService(requestedServiceName);

		// this tells the client who the worker is
		msg.Wrap(sender);

		// process MMI requests internally
		if (serviceFrame.strncmp("mmi.", 4)) {
			std::string returnCode;
			if (requestedServiceName == "mmi.service") {
				auto service_iterator = services.find(requestedServiceName);
				if (service_iterator != services.end()) {
					// 200 means OK and 404 means Not Found
					returnCode = service_iterator->second.numWorkers > 0 ? "200" : "404";
				}
			}
			else {
				// 501 means Method Not Supported
				returnCode = "501";
			}

			Frame clientReturnEnvelope = msg.Unwrap();
			std::string clientAddress = clientReturnEnvelope.GetHexData();
			clientReturnEnvelope.ReplaceData(returnCode);
			msg.Push(serviceFrame);
			msg.Push(Majordomo::ClientId);
			msg.Wrap(clientReturnEnvelope);
			if (verbose)
				printf("Broker: sending %d frames to client %s\n", (int)msg.Size(), clientAddress.c_str());
			msg.Send(brokerSocket);
		}
		else {
			DispatchService(service, msg);
		}

		return true;
	}

	void Broker::Purge() {
		auto worker_iterator = waitingWorkers.begin();
		while (worker_iterator != waitingWorkers.end()) {
			BrokerWorker* worker = *worker_iterator;
			if (zclock_time() < worker->expiry) {
				// Worker may still be alive
				break;
			}
			DeleteWorker(worker->name);
			worker_iterator = waitingWorkers.begin();
		}
	}

	void Broker::Run() {
		while (RunLoop())
			;
	}

	bool Broker::RunLoop() {
		if (!brokerSocket)
			return false;
		if (brokerSocket.Poll(Majordomo::DefaultHeartbeatIntervalPollMS)) {
			Message msg;
			msg.Recv(brokerSocket);

			if (msg.Size() >= 3) {
				if (verbose)
					printf("broker: received message size is %d\n", (int)msg.Size());
				Frame sender = msg.PopFrame();
				Frame empty = msg.PopFrame();
				Frame header = msg.PopFrame();

				if (header.GetStrData() == Majordomo::ClientId) {
					ProcessClientMessage(sender, msg);
				}
				else if (header.GetStrData() == Majordomo::WorkerId) {
					ProcessWorkerMessage(sender, msg);
				}
				else {
					// need to do error handling here...
					if (verbose)
						printf("broker: error with message\n");
				}
			}
			else {
				// need to do error handling here...
				if (verbose)
					printf("broker: error with message size\n");
			}
		}
		//else if (!brokerSocket)
		//{
		//	// socket interrupted
		//	if (verbose) printf("broker: socket interrupted\n");
		//	break;
		//}

		if (zclock_time() > heartbeatTime) {
			Purge();
			for (BrokerWorker* worker : waitingWorkers) {
				if (verbose)
					printf("broker: sending heartbeat to worker %s\n", worker->key.c_str());
				worker->SendHeartbeat(brokerSocket);
			}
			heartbeatTime = zclock_time() + Majordomo::DefaultHeartbeatIntervalMS;
		}

		return true;
	}

	bool Broker::WorkerExists(const std::string& workerName) const {
		const std::map<std::string, BrokerWorker>::const_iterator it = workers.find(workerName);

		return (it != workers.cend());
	}

	BrokerWorker* Broker::FindWorker(const std::string& workerName) {
		auto worker_iterator = workers.find(workerName);

		if (worker_iterator == workers.end()) {
			// worker not found
			workers[workerName].name = workerName;
			worker_iterator = workers.find(workerName);
		}

		return &worker_iterator->second;
	}

	void Broker::DisconnectWorker(BrokerWorker* worker) {
		if (verbose)
			printf("Broker: sending disconnect\n");
		worker->SendDisconnect(brokerSocket);
		DeleteWorker(worker->name);
	}

	void Broker::DeleteWorker(const std::string& workerName) {
		auto worker_iterator = workers.find(workerName);
		if (worker_iterator == workers.end())
			return;
		BrokerWorker* worker = &worker_iterator->second;

		if (worker->service) {
			worker->service->RemoveWorker(worker);
		}

		waitingWorkers.remove(worker);
		workers.erase(worker_iterator);
	}

	void Broker::WaitWorker(BrokerWorker* worker) {
		if (!worker)
			return;

		waitingWorkers.push_back(worker);
		worker->Wait();
		worker->service->AddWorker(worker);
		DispatchService(worker->service);
	}

	bool Broker::ServiceExists(const std::string& serviceName) const {
		auto it = services.find(serviceName);
		return (it != services.end());
	}

	BrokerServiceInfo* Broker::FindService(const std::string& serviceName) {
		auto service_iterator = services.find(serviceName);
		if (service_iterator == services.end()) {
			// service not found
			services[serviceName].name = serviceName;
			service_iterator = services.find(serviceName);
		}

		return &service_iterator->second;
	}

	void Broker::DispatchService(BrokerServiceInfo* service, Message& msg) {
		if (!service)
			return;

		if (msg)
			service->requests.push_back(msg);

		Purge();
		service->Dispatch(brokerSocket);
	}

	void Broker::DispatchService(BrokerServiceInfo* service) {
		Message empty;
		DispatchService(service, empty);
	}
} // namespace Uf
