#include "unicornfish_pch.hpp"
#include <unicornfish_worker.hpp>

namespace Uf {
	Worker::Worker() {}

	Worker::Worker(const char* endpoint, const char* service) {
		ConnectToBroker(endpoint, service);
	}

	Worker::~Worker() {}

	void Worker::Run() {
		Uf::Message reply;
		while (true) {
			if (SendReplyWaitRequest(reply) == false) {
				break;
			}
			OnProcessRequest(GetRequest(), reply);
		}
	}

	void Worker::OnProcessRequest(const Message& request, Message& reply) {
		reply = request;
	}

	bool Worker::ConnectToBroker(const char* endpoint, const char* service) {
		Disconnect();

		if (!workerSocket.NewDealer(endpoint)) {
			if (verbose)
				printf("worker: could not create socket\n");
			return false;
		}
		brokerEndpoint = endpoint;
		serviceName = service;

		Message msg(serviceName);
		if (!SendCommandToBroker(Majordomo::Command::Ready, msg))
			return false;

		// liveness = Majordomo::DefaultHearbeatAliveness;
		heartbeatTime = zclock_time() + Majordomo::DefaultHeartbeatDelayMS;

		return true;
	}

	void Worker::Disconnect() {
		if (workerSocket) {
			workerSocket.Delete();
		}
		serviceName.clear();
		brokerEndpoint.clear();
		replyMessage.Clear();
		requestMessage.Clear();
		heartbeatTime = Majordomo::DefaultHeartbeatIntervalMS;
		liveness = Majordomo::DefaultRetries;
		expectReply = false;
	}

	bool Worker::SendReplyWaitRequest(const Message& replyToSend) {
		replyMessage = replyToSend;
		if (replyMessage) {
			replyToHexAddress = replyTo.GetHexData();
			replyMessage.Wrap(replyTo);
			SendCommandToBroker(Majordomo::Command::Reply, replyMessage);
		}
		expectReply = true;
		auto t0 = zclock_mono();
		while (true) {
			if (workerSocket.Poll(Majordomo::DefaultHeartbeatIntervalPollMS)) {

				requestMessage.Recv(workerSocket);
				if (verbose)
					printf("worker: received message size is %d\n", (int)requestMessage.Size());
				liveness = Majordomo::DefaultHearbeatAliveness;

				if (requestMessage.Size() >= 3) {
					bool poppedEmpty = requestMessage.PopEmpty();
					std::string header = requestMessage.PopString();
					Majordomo::Command command = requestMessage.PopCommand();

					if (poppedEmpty == false || header != Majordomo::WorkerId) {
						return false;
					}
					if (command == Majordomo::Command::Request) {
						replyTo = requestMessage.Unwrap();
						return requestMessage;
					}
					else if (command == Majordomo::Command::Heartbeat) {
						// do nothing for heartbeat
						if (verbose)
							printf("worker: received heartbeat\n");
					}
					else if (command == Majordomo::Command::Disconnect) {
						// we got disconnected, so try to connect again
						if (verbose)
							printf("worker: received disconnect, trying to reconnect\n");
						ConnectToBroker();
					}
					else {
						// invalid message
					}
				}
			}
			else if (!workerSocket) {
				// got interrupted
				if (verbose)
					printf("worker: socket was interrupted\n");
				return false;
			}
			else if (--liveness == 0) {
				if (verbose)
					printf("worker: disconnected from broker, retrying to connect...\n");
				//retries--;
				//if (retries <= 0) return false;
				zclock_sleep(Majordomo::DefaultReconnectDelayMS);
				if (!ConnectToBroker())
					return false;
			}
			if (zclock_time() > heartbeatTime) {
				SendCommandToBroker(Majordomo::Command::Heartbeat);
				heartbeatTime = zclock_time() + Majordomo::DefaultHeartbeatIntervalMS;
			}
			if (zclock_mono() - t0 > 10000)
				return false;
		}
		return true;
	}

	bool Worker::SendReply(const Message& replyToSend) {
		replyMessage = replyToSend;
		if (replyMessage) {
			replyToHexAddress = replyTo.GetHexData();
			replyMessage.Wrap(replyTo);
			SendCommandToBroker(Majordomo::Command::Reply, replyMessage);
		}
		expectReply = true;
		return true;
	}

	bool Worker::WaitRequest() {
		auto t0 = zclock_mono();
		while (true) {
			if (workerSocket.Poll(Majordomo::DefaultHeartbeatIntervalPollMS)) {
				requestMessage.Recv(workerSocket);
				if (verbose)
					printf("worker: received message size is %d\n", (int)requestMessage.Size());
				liveness = Majordomo::DefaultHearbeatAliveness;

				if (requestMessage.Size() >= 3) {
					bool poppedEmpty = requestMessage.PopEmpty();
					std::string header = requestMessage.PopString();
					Majordomo::Command command = requestMessage.PopCommand();

					if (poppedEmpty == false || header != Majordomo::WorkerId) {
						return false;
					}
					if (command == Majordomo::Command::Request) {
						replyTo = requestMessage.Unwrap();
						return requestMessage;
						return true;
					}
					else if (command == Majordomo::Command::Heartbeat) {
						// do nothing for heartbeat
						if (verbose)
							printf("worker: received heartbeat\n");
					}
					else if (command == Majordomo::Command::Disconnect) {
						// we got disconnected, so try to connect again
						if (verbose)
							printf("worker: received disconnect, trying to reconnect\n");
						ConnectToBroker();
					}
					else {
						// invalid message
					}
				}
			}
			else if (!workerSocket) {
				// got interrupted
				if (verbose)
					printf("worker: socket was interrupted\n");
				return false;
			}
			else if (--liveness == 0) {
				if (verbose)
					printf("worker: disconnected from broker, retrying to connect...\n");
				//retries--;
				//if (retries <= 0) return false;
				zclock_sleep(Majordomo::DefaultReconnectDelayMS);
				if (!ConnectToBroker())
					return false;
			}
			if (zclock_time() > heartbeatTime) {
				SendCommandToBroker(Majordomo::Command::Heartbeat);
				heartbeatTime = zclock_time() + Majordomo::DefaultHeartbeatIntervalMS;
			}
			if (zclock_mono() - t0 > 10000)
				return false;
		}
		return false;
	}

	bool Worker::ConnectToBroker() {
		std::string save_endpoint = brokerEndpoint;
		std::string save_service = serviceName;
		Disconnect();
		return ConnectToBroker(save_endpoint.c_str(), save_service.c_str());
	}

	bool Worker::SendCommandToBroker(Majordomo::Command command) {
		Message msg;
		msg.Create();
		return SendCommandToBroker(command, msg);
	}

	bool Worker::SendCommandToBroker(Majordomo::Command command, Message& msg) {
		if (!msg)
			msg.Create();
		msg.Push(command);
		msg.Push(Majordomo::WorkerId);
		msg.Push("");
		if (verbose)
			printf("worker: sending %d frames to broker for client %s \n", (int)msg.Size(), replyToHexAddress.c_str());
		return msg.Send(workerSocket);
	}

	bool Worker::SendCommandToClient(Majordomo::Command command, Message& msg) {
		if (!msg)
			msg.Create();
		msg.Push(command);
		msg.Push(Majordomo::ClientId);
		if (verbose)
			printf("worker: sending %d frames to client %s \n", (int)msg.Size(), replyToHexAddress.c_str());
		return msg.Send(workerSocket);
	}
} // namespace Uf
