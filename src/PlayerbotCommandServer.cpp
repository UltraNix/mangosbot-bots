/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotCommandServer.h"

#include <cstdlib>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#if BOOST_VERSION >= 106600
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#else
#include <boost/asio/io_service.hpp>
#endif

using boost::asio::ip::tcp;
typedef boost::shared_ptr<tcp::socket> socket_ptr;

bool ReadLine(socket_ptr sock, std::string* buffer, std::string* line)
{
    // Do the real reading from fd until buffer has '\n'.
    std::string::iterator pos;
    while ((pos = find(buffer->begin(), buffer->end(), '\n')) == buffer->end())
    {
        char buf[1025];
        boost::system::error_code error;
        size_t n = sock->read_some(boost::asio::buffer(buf), error);
        if (n == -1 || error == boost::asio::error::eof)
            return false;
        else if (error)
            throw boost::system::system_error(error); // Some other error.

        buf[n] = 0;
        *buffer += buf;
    }

    *line = std::string(buffer->begin(), pos);
    *buffer = std::string(pos + 1, buffer->end());
    return true;
}

void session(socket_ptr sock)
{
    try
    {
        std::string buffer, request;
        while (ReadLine(sock, &buffer, &request))
        {
            std::string response = sRandomPlayerbotMgr->HandleRemoteCommand(request) + "\n";
            boost::asio::write(*sock, boost::asio::buffer(response.c_str(), response.size()));
            request = "";
        }
    }
    catch (std::exception& e)
    {
        LOG_ERROR("playerbots", "%s", e.what());
    }
}

#if BOOST_VERSION >= 106600
void server(boost::asio::io_context& io_service, short port)
#else
void server(boost::asio::io_service& io_service, short port)
#endif
{
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
    for (;;)
    {
        socket_ptr sock(new tcp::socket(io_service));
        a.accept(*sock);
        boost::thread t(boost::bind(session, sock));
    }
}

void Run()
{
    if (!sPlayerbotAIConfig->commandServerPort)
    {
        return;
    }

    std::ostringstream s;
    s << "Starting Playerbot Command Server on port " << sPlayerbotAIConfig->commandServerPort;
    LOG_INFO("playerbots", "%s", s.str().c_str());

    try
    {
#if BOOST_VERSION >= 106600
        boost::asio::io_context io_service;
#else
        boost::asio::io_service io_service;
#endif
        server(io_service, sPlayerbotAIConfig->commandServerPort);
    }

    catch (std::exception& e)
    {
        LOG_ERROR("playerbots", "%s", e.what());
    }
}

void PlayerbotCommandServer::Start()
{
    std::thread serverThread(Run);
    serverThread.detach();
}
