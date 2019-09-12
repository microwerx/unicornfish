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

#include <unicornfish_message.hpp>

namespace Uf
{
Message::Message()
{
    // Create();
}

Message::Message(const std::string &message)
{
    if (Create())
        Push(message);
}

Message::Message(const char *message)
{
    if (Create())
        Push(message);
}

Message::Message(const Majordomo::Command command)
{
    if (Create())
        Push(command);
}

Message::Message(void *data, size_t size)
{
    if (Create())
        Push(data, size);
}

Message::Message(const Message &msgToCopy)
{
    msg = zmsg_dup(msgToCopy.msg);
}

Message::~Message()
{
    Delete();
}

const Message &Message::operator=(const Message &msgToCopy)
{
    Delete();
    msg = zmsg_dup(msgToCopy.msg);
    return *this;
}

bool Message::Create()
{
    Delete();
    msg = zmsg_new();
    if (msg != nullptr)
        return true;
    return false;
}

void Message::Move(zmsg_t **msgToMove)
{
    Delete();
    msg = *msgToMove;
    *msgToMove = nullptr;
}

void Message::Copy(const zmsg_t *msgToCopy)
{
    Delete();
    msg = zmsg_dup(const_cast<zmsg_t *>(msgToCopy));
}

void Message::Clear()
{
    Delete();
    Create();
}

bool Message::Push(const std::string &message)
{
    if (zmsg_pushstr(msg, message.c_str()) == 0)
        return true;
    return false;
}

bool Message::Push(const char *message)
{
    if (!msg || !message)
        return false;
    if (zmsg_pushstr(msg, message) == 0)
        return true;
    return false;
}

bool Message::Push(const Majordomo::Command command)
{
    if (!msg)
        return false;
    char message[2] = {static_cast<char>(command), '\0'};
    if (zmsg_pushstr(msg, message) == 0)
        return true;
    return false;
}

bool Message::Push(const Frame &frame)
{
    if (!frame || !msg)
        return false;
    Frame copiedFrame;
    copiedFrame.Copy(frame);
    if (zmsg_prepend(msg, &copiedFrame.frame) == 0)
        return true;
    return false;
}

bool Message::Push(void *data, size_t size)
{
    if (!msg)
        return false;
    if (zmsg_pushmem(msg, data, size) == 0)
        return true;
    return false;
}

bool Message::PopEmpty()
{
    if (!msg)
        return false;
    zframe_t *empty = zmsg_pop(msg);
    if (empty != nullptr && zframe_streq(empty, ""))
    {
    }
    zframe_destroy(&empty);
    return true;
}

int Message::PopInt8()
{
    if (!msg)
        return 0;
    char *str = zmsg_popstr(msg);
    int returnValue = str[0];
    lastPoppedString = str;
    zstr_free(&str);
    return returnValue;
}

Majordomo::Command Message::PopCommand()
{
    using namespace Majordomo;
    if (!msg)
        return Command::None;
    char *str = zmsg_popstr(msg);
    int value = str[0];
    lastPoppedString = str;
    zstr_free(&str);
    if (value == (int)Command::None)
        return Command::None;
    if (value == (int)Command::Ready)
        return Command::Ready;
    if (value == (int)Command::Request)
        return Command::Request;
    if (value == (int)Command::Reply)
        return Command::Reply;
    if (value == (int)Command::Heartbeat)
        return Command::Heartbeat;
    if (value == (int)Command::Disconnect)
        return Command::Disconnect;
    return Command::None;
}

Frame &Message::PopFrame()
{
    lastPoppedFrame.Delete();
    if (!msg)
        return lastPoppedFrame;
    zframe_t *frame = zmsg_pop(msg);
    lastPoppedFrame.Move(&frame);
    return lastPoppedFrame;
}

const std::string &Message::PopString()
{
    lastPoppedString.clear();
    if (!msg)
        return lastPoppedString;
    char *str = zmsg_popstr(msg);
    if (str != nullptr)
    {
        lastPoppedString = str;
        zstr_free(&str);
    }
    else
    {
        lastPoppedString.clear();
    }
    return lastPoppedString;
}

void Message::PopMem(void *data, size_t size)
{
    if (!msg)
        return;
    Frame &frame = PopFrame();
    memcpy(data, frame.GetData(), std::min(size, frame.SizeInBytes()));
}

const std::string &Message::GetLastFrameString()
{
    lastFrameString.clear();
    if (!msg)
        return lastFrameString;
    zframe_t *last = zmsg_last(msg);
    char *str = last ? zframe_strdup(last) : nullptr;
    if (str != nullptr)
    {
        lastFrameString = str;
        zstr_free(&str);
    }
    else
    {
        lastFrameString.clear();
    }
    return lastFrameString;
}

bool Message::Wrap(const Frame &frame)
{
    if (!msg || !frame)
        return false;
    Frame copiedFrame;
    copiedFrame.Copy(frame);
    if (zmsg_pushmem(msg, "", 0) == 0)
        zmsg_prepend(msg, &copiedFrame.frame);
    return true;
}

Frame &Message::Unwrap()
{
    lastUnwrappedFrame.Delete();
    if (!msg)
        return lastUnwrappedFrame;

    zframe_t *frame = zmsg_pop(msg);
    zframe_t *empty = zmsg_first(msg);
    if (empty && zframe_size(empty) == 0)
    {
        empty = zmsg_pop(msg);
        zframe_destroy(&empty);
    }

    lastUnwrappedFrame.Move(&frame);

    return lastUnwrappedFrame;
}

bool Message::Send(Socket &socket)
{
    //printf("Message::Send() -- sending %d frames\n", Size());
    //zmsg_dump(msg);
    if (Size() == 0)
        return false;
    return socket.SendMessage(*this);
}

bool Message::Recv(Socket &socket)
{
    bool result = socket.RecvMessage(*this);
    //printf("Message::Recv() -- got %d frames\n", Size());
    //zmsg_dump(msg);
    if (Size() == 0)
        return false;
    return result;
}

bool Message::SetLastFrameData(const std::string &data)
{
    if (!msg)
        return false;
    zframe_reset(zmsg_last(msg), data.c_str(), data.size() + 1);
    return true;
}

bool Message::SetLastFrameData(const char *data)
{
    zframe_reset(zmsg_last(msg), data, strlen(data));
    return true;
}

size_t Message::Size() const
{
    if (!msg)
        return 0;
    return zmsg_size(msg);
}

void Message::Delete()
{
    if (msg != nullptr)
    {
        zmsg_destroy(&msg);
        msg = nullptr;
    }
}
} // namespace Uf