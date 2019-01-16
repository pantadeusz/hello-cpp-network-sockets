/**
 * @file server.cpp
 * @author Tadeusz Puźniakowski
 * @brief This is verry basic server implementation in C++17
 * @version 0.1
 * @date 2019-01-16
 *
 * @copyright Copyright (c) 2019 Tadeusz Puźniakowski
 * @license MIT
 */

// for network code
#include <netdb.h>
#include <unistd.h>

// for functional code
#include <functional>

// for the main function
#include <iostream>
#include <vector>

/**
 * @brief create listening socket
 *
 * @param success callback that receive bind-ed socket
 * @param error callback on error
 * @param server_name the address on which we should listen
 * @param port_name port on which we listen
 * @param max_queue the number of waiting connections
 */
int listen_server(std::function<void(int)> success,
                  std::function<void(const char *)> error = [](auto e) {},
                  const char *server_name = "0.0.0.0",
                  const char *port_name = "9921", const int max_queue = 32) {
  int listening_socket;
  struct addrinfo hints;
  std::fill((char *)&hints, (char *)&hints + sizeof(struct addrinfo), 0);

  hints.ai_family = AF_UNSPEC;     ///< IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; ///< Stream socket
  hints.ai_flags = AI_PASSIVE;     ///< For wildcard IP address
  hints.ai_protocol = 0;           ///< Any protocol
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  struct addrinfo *result, *rp;
  if (int s = getaddrinfo(server_name, port_name, &hints, &result); s != 0) {
    error(gai_strerror(s));
    return -1;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    listening_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (listening_socket != -1) {
      if (int yes = 1; setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR,
                                  &yes, sizeof(yes)) == -1) {
        error("setsockopt( ... ) error");
        return -1;
      }
      if (bind(listening_socket, rp->ai_addr, rp->ai_addrlen) == 0) {
        freeaddrinfo(result);
        if (listen(listening_socket, max_queue) == -1) {
          error("listen error");
          return -1;
        }
        success(listening_socket);
        return listening_socket;
      }
      ::close(listening_socket);
    }
  }
  freeaddrinfo(result);
  error("error binding adress");
  return -1;
}

/**
 * @brief function that performs accepting of connections.
 *
 * @param listening_socket the correct listening socket
 * @param success callback that will get (connected socket, connected host name,
 * connected service name)
 * @param error callback on error
 *
 * @return connected or failed socket descriptor
 */
int do_accept(int listening_socket,
              std::function<void(int, std::string, std::string)>
                  success, /* connected_socket, host, service */
              std::function<void(const char *)> error = [](auto e) {}) {
  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
  int connected_socket;

  if ((connected_socket =
           ::accept(listening_socket, (struct sockaddr *)&peer_addr,
                    &peer_addr_len)) == -1) {
    error("could not accept connection!");
    return connected_socket;
  } else {
    char host[NI_MAXHOST], service[NI_MAXSERV];
    getnameinfo((struct sockaddr *)&peer_addr, peer_addr_len, host, NI_MAXHOST,
                service, NI_MAXSERV, NI_NUMERICSERV);
    success(connected_socket, host, service);
    return connected_socket;
  }
}

int main(int argc, char **argv) {
  std::vector<char *> args(argv, argv + argc);

  listen_server(
      [](int listening_socket) {
        std::cout << "Waiting for connection..." << std::endl;
        do_accept(listening_socket, [&](int s, auto h, auto p) {
          char msg[100];
          ::read(s, msg, 100);
          std::cout << "received \"" << msg << "\" from " << h
                    << " connected on " << p << std::endl;
          ::close(s);
        });
        ::close(listening_socket);
      },
      [](auto err) { std::cout << "Error connecting: " << err << std::endl; },
      "localhost", (args.size() > 1) ? args[1] : "9921", 2);
  return 0;
}
