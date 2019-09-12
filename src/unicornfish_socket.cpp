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

#include <unicornfish_socket.hpp>
// #include <varargs.h>

namespace Uf
{
Socket::Socket()
{
}

Socket::~Socket()
{
    Delete();
}

void Socket::Delete()
{
    if (socket == nullptr)
        return;

    zsock_destroy(&socket);
    socket = nullptr;
    if (poller)
        zpoller_destroy(&poller);
    poller = nullptr;

    m_endpoint.clear();
    m_subPrefix.clear();
    m_socketType = SocketType::NONE;
}

bool Socket::SetupSocket(std::string endpoint, std::string subPrefix, SocketType socketType)
{
    if (!socket)
        return false;
    poller = zpoller_new(socket, NULL);
    if (!poller)
        return false;
    m_endpoint = endpoint;
    m_subPrefix = subPrefix;
    m_socketType = socketType;
    return true;
}

bool Socket::NewPub(const std::string &endpoint)
{
    Delete();

    socket = zsock_new_pub(endpoint.c_str());

    if (!socket)
        return false;

    m_socketType = SocketType::PUB;
    m_endpoint = endpoint;
    return true;
}

bool Socket::NewSub(const std::string &endpoint, const std::string &subPrefix)
{
    Delete();

    socket = zsock_new_sub(endpoint.c_str(), subPrefix.c_str());

    if (!socket)
        return false;

    m_socketType = SocketType::SUB;
    m_endpoint = endpoint;
    m_subPrefix = subPrefix;
    return true;
}

bool Socket::NewReq(const std::string &endpoint)
{
    Delete();

    socket = zsock_new_req(endpoint.c_str());
    if (!socket)
        return false;

    m_socketType = SocketType::REQ;
    m_endpoint = endpoint;
    return true;
}

bool Socket::NewRep(const std::string &endpoint)
{
    Delete();

    socket = zsock_new_rep(endpoint.c_str());
    if (!socket)
        return false;

    m_socketType = SocketType::REP;
    m_endpoint = endpoint;
    return true;
}

bool Socket::NewDealer(const std::string &endpoint)
{
    Delete();

    socket = zsock_new_dealer(endpoint.c_str());
    if (!socket)
        return false;

    return SetupSocket(endpoint, "", SocketType::DEALER);
}

bool Socket::NewRouter(const std::string &endpoint)
{
    Delete();

    socket = zsock_new_router(endpoint.c_str());
    if (!socket)
        return false;

    return SetupSocket(endpoint, "", SocketType::ROUTER);
}

bool Socket::NewPush(const std::string &endpoint)
{
    Delete();

    socket = zsock_new_push(endpoint.c_str());
    if (!socket)
        return false;

    return SetupSocket(endpoint, "", SocketType::PUSH);
}

bool Socket::NewPull(const std::string &endpoint)
{
    Delete();

    socket = zsock_new_pull(endpoint.c_str());
    if (!socket)
        return false;

    return SetupSocket(endpoint, "", SocketType::PULL);
}

bool Socket::NewXPub(const std::string &endpoint)
{
    Delete();

    socket = zsock_new_xpub(endpoint.c_str());

    if (!socket)
        return false;

    return SetupSocket(endpoint, "", SocketType::XPUB);
}

bool Socket::NewXSub(const std::string &endpoint)
{
    Delete();

    socket = zsock_new_xsub(endpoint.c_str());

    if (!socket)
        return false;

    return SetupSocket(endpoint, "", SocketType::XSUB);
}

bool Socket::NewPair(const std::string &endpoint)
{
    Delete();

    socket = zsock_new_pair(endpoint.c_str());
    if (!socket)
        return false;

    return SetupSocket(endpoint, "", SocketType::PAIR);
}

bool Socket::NewStream(const std::string &endpoint)
{
    Delete();

    socket = zsock_new_stream(endpoint.c_str());
    if (!socket)
        return false;

    return SetupSocket(endpoint, "", SocketType::STREAM);
}

bool Socket::Signal(unsigned char status)
{
    if (zsock_signal(socket, status) == 0)
        return true;
    return false;
}

bool Socket::Wait()
{
    if (zsock_wait(socket) == 0)
        return true;
    return false;
}

void Socket::Flush()
{
    zsock_flush(socket);
}

bool Socket::Poll(long timeout_ms)
{
    //zmq_pollitem_t items[] = {
    //	{ socket, 0, ZMQ_POLLIN, 0 }
    //};

    //if (zmq_poll(items, 1, timeout_ms * ZMQ_POLL_MSEC) == -1)
    //{
    //	// interrupted, so delete socket
    //	Delete();
    //	return false;
    //}

    //if (items[0].revents & ZMQ_POLLIN)
    //	return true;

    if (poller)
    {
        zsock_t *polled_socket = (zsock_t *)zpoller_wait(poller, timeout_ms);
        if (polled_socket == nullptr)
        {
            if (zpoller_terminated(poller))
            {
                Delete();
                return false;
            }
            else if (zpoller_expired(poller))
            {
                return false;
            }
        }
        else
        {
            return true;
        }
    };

    return false;
}

bool Socket::Send(const char *picture, ...)
{
    va_list args;
    va_start(args, picture);

    if (zsock_vsend(socket, picture, args) == 0)
        return true;

    return false;
}

bool Socket::Recv(const char *picture, ...)
{
    va_list args;
    va_start(args, picture);

    if (zsock_vrecv(socket, picture, args) == 0)
        return true;

    return false;
}

bool Socket::SendMessage(Message &msg)
{
    zmsg_t *send_msg = msg.GetZmsg();

    if (socket == nullptr || send_msg == nullptr)
        return false;

    if (zsock_send(socket, "m", send_msg) == 0)
        return true;

    return false;
}

bool Socket::RecvMessage(Message &msg)
{
    if (socket == nullptr)
        return false;

    zmsg_t *recv_msg;
    if (zsock_recv(socket, "m", &recv_msg) == 0)
    {
        msg.Move(&recv_msg);
        return true;
    }

    return false;
}
} // namespace Uf