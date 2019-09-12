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
#ifndef UNICORNFISH_MESSAGE_HPP
#define UNICORNFISH_MESSAGE_HPP

#include <unicornfish.hpp>
#include <unicornfish_socket.hpp>
#include <unicornfish_frame.hpp>
#include <unicornfish_majordomo.hpp>

namespace Uf
{
class Socket;

class Message
{
  public:
	Message();
	Message(const std::string &message);
	Message(const char *message);
	Message(const Majordomo::Command command);
	Message(void *data, size_t size);
	Message(const Message &msgToCopy);
	~Message();

	const Message &operator=(const Message &msgToCopy);

	bool Create();
	bool Create(const std::string &message)
	{
		if (Create())
			return Push(message);
		return false;
	}

	void Move(zmsg_t **msgToMove);
	void Copy(const zmsg_t *msgToCopy);

	void Delete();
	operator bool() const { return msg != nullptr; }
	void Clear();

	bool Wrap(const Frame &frame);
	Frame &Unwrap();

	bool Send(Socket &socket);
	bool Recv(Socket &socket);

	bool SetLastFrameData(const std::string &data);
	bool SetLastFrameData(const char *data);

	inline bool PushStr(const std::string &message) { return Push(message); }
	inline bool PushMem(void *mem, size_t size) { return Push(mem, size); }
	inline bool PushFrame(const Frame &frame) { return Push(frame); }

	bool Push(const std::string &message);
	bool Push(const char *message);
	bool Push(const Majordomo::Command command);
	bool Push(const Frame &frame);
	bool Push(void *mem, size_t size);
	bool PopEmpty();
	int PopInt8();
	Majordomo::Command PopCommand();
	Frame &PopFrame();
	const std::string &PopString();
	void PopMem(void *data, size_t size);
	size_t Size() const;
	bool Empty() const { return Size() == 0; }
	const std::string &GetLastFrameString();

	zmsg_t *GetZmsg() { return msg; }

  private:
	zmsg_t *msg = nullptr;
	std::string lastPoppedString;
	std::string lastFrameString;
	Frame lastPoppedFrame;
	Frame lastUnwrappedFrame;
};
} // namespace Uf

#endif