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

#include <unicornfish_client.hpp>
#include <unicornfish_majordomo.hpp>

namespace Uf
{
	Client::Client()
	{
	}

	Client::Client(const std::string &endpoint)
	{
		ConnectToBroker(endpoint);
	}

	Client::Client(const char* endpoint)
	{
		ConnectToBroker(endpoint);
	}

	Client::~Client()
	{
		Disconnect();
	}

	bool Client::ConnectToBroker(const std::string &endpoint)
	{
		return ConnectToBroker(endpoint.c_str());
	}

	bool Client::ConnectToBroker(const char* endpoint)
	{
		clientSocket.NewDealer(endpoint);
		brokerEndpoint = endpoint;

		return true;
	}

	void Client::Disconnect()
	{
		if (clientSocket) {
			clientSocket.Delete();
		}
	}

	void Client::ClearReply()
	{
		replyHeader.clear();
		replyService.clear();
		replyMessage.Clear();
	}

	bool Client::SendRequest(const std::string &service, const Message& request)
	{
		return SendRequestToBroker(service.c_str(), request);
	}

	bool Client::SendRequest(const char* service, const Message& request)
	{
		return SendRequestToBroker(service, request);
	}

	bool Client::PollReply()
	{
		if (clientSocket.Poll(Majordomo::DefaultTimeoutMS)) {
			return true;
		}
		return false;
	}

	bool Client::WaitReply()
	{
		int retriesLeft = this->requestRetries;
		while (retriesLeft) {
			if (clientSocket.Poll(Majordomo::DefaultTimeoutMS)) {
				replyMessage.Recv(clientSocket);
				if (verbose)
					printf("client: received got %d frames\n", (int)replyMessage.Size());

				if (replyMessage.Size() >= 3) {
					replyHeader = replyMessage.PopString();
					if (replyHeader == Majordomo::ClientId) {
						replyService = replyMessage.PopString();
					}
				}
				return true;
			}
			else if (!clientSocket) {
				if (verbose)
					printf("client: socket failed!\n");
				return false;
			}
			else if (--retriesLeft > 0) {
				ConnectToBroker(brokerEndpoint.c_str());
			}
			else {
				if (verbose)
					printf("client: failed to connect to broker\n");
				return false;
			}
		}
		return true;
	}

	bool Client::SendWaitReply(const std::string &service, const Message& request)
	{
		return SendWaitReply(service.c_str(), request);
	}

	bool Client::SendWaitReply(const char* service, const Message& request)
	{
		if (!SendRequestToBroker(service, request))
			return false;

		int retriesLeft = this->requestRetries;
		while (retriesLeft) {
			SendRequestToBroker(service, request);

			if (clientSocket.Poll(Majordomo::DefaultTimeoutMS)) {
				replyMessage.Recv(clientSocket);
				if (verbose)
					printf("client: received got %d frames\n", (int)replyMessage.Size());

				if (replyMessage.Size() >= 3) {
					replyHeader = replyMessage.PopString();
					if (replyHeader == Majordomo::ClientId) {
						replyService = replyMessage.PopString();
					}
				}
				return true;
			}
			else if (!clientSocket) {
				if (verbose)
					printf("client: socket failed!\n");
				return false;
			}
			else if (--retriesLeft > 0) {
				ConnectToBroker(brokerEndpoint.c_str());
			}
			else {
				if (verbose)
					printf("client: failed to connect to broker\n");
				return false;
			}
		}
		return true;
	}

	bool Client::SendRequestToBroker(const std::string &service, const Message& msg)
	{
		return SendRequestToBroker(service.c_str(), msg);
	}

	bool Client::SendRequestToBroker(const char* service, const Message& msg)
	{
		if (msg.Size() < 1)
			return false;

		Message requestMessage = msg;
		requestMessage.Push(service);
		requestMessage.Push(Majordomo::ClientId);
		requestMessage.Push("");
		if (verbose)
			printf("client: sending %d frames to broker\n", (int)requestMessage.Size());
		return requestMessage.Send(clientSocket);
	}
}