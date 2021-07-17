/**
 * @file server.cpp
 * @author Tadeusz Puźniakowski
 * @brief This is verry basic server implementation in C++17
 * @version 0.2
 * @date 2021-07-17
 *
 * @copyright Copyright (c) 2019 Tadeusz Puźniakowski
 * @license MIT
 * 
 * BUILD
 * g++ -std=c++17 -fopenmp server.cpp -o server
 */

// for network code
#include <netdb.h>
#include <unistd.h>

// for the main function
#include <future>
#include <iostream>
#include <vector>

/**
 * @brief create listening socket
 *
 * @param server_name the address on which we should listen
 * @param port_name port on which we listen
 * @param max_queue the number of waiting connections
 */
std::future<int> listen_server(
    const char* server_name = "0.0.0.0",
    const char* port_name = "9921",
    const int max_queue = 32)
{
    return std::async(std::launch::async, [=]() -> int {
        int listening_socket;
        struct addrinfo hints;
        std::fill((char*)&hints, (char*)&hints + sizeof(struct addrinfo), 0);

        hints.ai_family = AF_UNSPEC;     ///< IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; ///< Stream socket
        hints.ai_flags = AI_PASSIVE;     ///< For wildcard IP address
        hints.ai_protocol = 0;           ///< Any protocol
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;

        struct addrinfo *result, *rp;
        if (int s = getaddrinfo(server_name, port_name, &hints, &result); s != 0) {
            throw std::invalid_argument(gai_strerror(s));
        }

        for (rp = result; rp != NULL; rp = rp->ai_next) {
            listening_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (listening_socket != -1) {
                if (int yes = 1; setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR,
                                     &yes, sizeof(yes)) == -1) {
                    throw std::invalid_argument("setsockopt( ... ) error");
                }
                if (bind(listening_socket, rp->ai_addr, rp->ai_addrlen) == 0) {
                    freeaddrinfo(result);
                    if (listen(listening_socket, max_queue) == -1) {
                        throw std::invalid_argument("listen error");
                    }
                    return listening_socket;
                }
                ::close(listening_socket);
            }
        }
        freeaddrinfo(result);
        throw std::invalid_argument("error binding adress");
    });
}

/**
 * @brief function that performs accepting of connections.
 *
 * @param listening_socket the correct listening socket
 * @return {connected or failed socket descriptor, hostname, socket}
 */
std::tuple<int, std::string, std::string> do_accept(
    int listening_socket)
{
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
    int connected_socket;

    if ((connected_socket =
                ::accept(listening_socket, (struct sockaddr*)&peer_addr,
                    &peer_addr_len)) == -1) {
        throw std::invalid_argument("could not accept connection!");
    } else {
        char host[NI_MAXHOST], service[NI_MAXSERV];
        getnameinfo((struct sockaddr*)&peer_addr, peer_addr_len, host, NI_MAXHOST,
            service, NI_MAXSERV, NI_NUMERICSERV);
        return {connected_socket, host, service};
    }
}

int main(int argc, char** argv)
{
    std::vector<char*> args(argv, argv + argc);

    auto listening_socket_fut =
        listen_server("*",
            (args.size() > 1) ? args[1] : "9921", 2);

    int listening_socket = listening_socket_fut.get();
    std::cout << "Waiting for connection..." << std::endl;
    auto [s, h, p] = do_accept(listening_socket);
    char msg[100];
    ::read(s, msg, 100);
    std::cout << "received \"" << msg << "\" from " << h << " connected on "
              << p << std::endl;
    ::close(s);
    ::close(listening_socket);
    return 0;
}
