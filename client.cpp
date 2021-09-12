/**
 * @file client.cpp
 * @author Tadeusz Puźniakowski
 * @brief This is verry basic client implementation in C++17
 * @version 0.2
 * @date 2021-07-17
 *
 * @copyright Copyright (c) 2019 Tadeusz Puźniakowski
 * @license MIT
 * 
 * BUILD:
 * g++ -std=c++17 -fopenmp client.cpp -o client
 */

// for network code
#include <netdb.h>
#include <unistd.h>

// for main example
#include <future>
#include <iostream>
#include <memory>
#include <vector>

/**
 * @brief opens a connection to given host and port
 *
 * @param addr_txt
 * @param port_txt
 *
 * @return future returning connected socket. If error, there will be exception
 */
std::future<int> connect_to(const char* addr_txt, const char* port_txt)
{
    return std::async(std::launch::async, [=]() -> int {
        struct addrinfo hints = {};
        hints.ai_family = AF_UNSPEC;     ///< IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; ///< stream socket

        struct addrinfo* addr_p;
        if (int err = getaddrinfo(addr_txt, port_txt, &hints, &addr_p); err) {
            throw std::invalid_argument(gai_strerror(err));
        }
        struct addrinfo* rp;
        for (rp = addr_p; rp != NULL; rp = rp->ai_next) {
            int connected_socket =
                socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (connected_socket != -1) {
                if (connect(connected_socket, rp->ai_addr, rp->ai_addrlen) != -1) {
                    freeaddrinfo(addr_p);
                    return connected_socket;
                }
                ::close(connected_socket);
            }
        }
        freeaddrinfo(addr_p);
        throw std::invalid_argument("could not open connection");
    });
}

/**
 * @brief The main function. Arguments can be [host] [port] [message to send]
 *
 * It shows how to use the connect_to function
 */

int main(int argc, char** argv)
{
    std::vector<char*> args(argv, argv + argc);

    // let's try to connect. it will return future, so we can do some other tasks in parallel
    auto fut = connect_to((args.size() > 1) ? args[1] : "localhost",
        (args.size() > 2) ? args[2] : "9921");
    // when we need the connected socket, just get it. Watch out for exceptions
    try {
        int s = fut.get();
        // prepare some message
        std::string msg =
            (args.size() > 3) ? args[3] : "Hi! How are you?"; // we can send also custom messages
        std::vector<char> msg_to_send(msg.begin(), msg.end());
        msg_to_send.resize(100);
        // send it to the other side
        ::write(s, msg_to_send.data(), 100);
        // and close the connection
        ::close(s);
    } catch (const std::exception& e) {
        std::cout << "Error connecting: " << e.what() << '\n';
    }

    return 0;
}
