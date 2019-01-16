/**
 * @file client.cpp
 * @author Tadeusz Puźniakowski
 * @brief This is verry basic client implementation in C++17
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

// for main example
#include <iostream>
#include <memory>
#include <vector>

/**
 * @brief opens a connection to given host and port
 *
 * @param addr_txt
 * @param port_txt
 * @param success
 * @param error
 *
 * @return connected socket
 */
int connect_to(const char *addr_txt, const char *port_txt,
               std::function<void(int)> success,
               std::function<void(const char *)> error = [](auto e) {}) {

  struct addrinfo hints;
  std::fill((char *)&hints, (char *)&hints + sizeof(struct addrinfo), 0);
  hints.ai_family = AF_UNSPEC;     ///< IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; ///< stream socket
  hints.ai_flags = 0;              ///< no additional flags
  hints.ai_protocol = 0;           ///< any protocol

  struct addrinfo *addr_p;
  if (int err = getaddrinfo(addr_txt, port_txt, &hints, &addr_p); err) {
    error(gai_strerror(err));
    return -1;
  }
  struct addrinfo *rp;
  for (rp = addr_p; rp != NULL; rp = rp->ai_next) {
    int connected_socket =
        socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (connected_socket != -1) {
      if (connect(connected_socket, rp->ai_addr, rp->ai_addrlen) != -1) {
        freeaddrinfo(addr_p);
        success(connected_socket);
        return connected_socket;
      }
      ::close(connected_socket);
    }
  }
  freeaddrinfo(addr_p);
  error("could not open connection");
  return -1;
}

/**
 * @brief The main function. Arguments can be [host] [port] [message to send]
 *
 * It shows how to use the connect_to function
 */

int main(int argc, char **argv) {
  std::vector<char *> args(argv, argv + argc);

  auto on_connected = [&](int s) {
    std::string msg =
        (args.size() > 3)
            ? args[3]
            : "Hi! How are you?"; // we can send also custom messages
    std::vector<char> msg_to_send(msg.begin(), msg.end());
    msg_to_send.resize(100);
    ::write(s, msg_to_send.data(), 100);
    ::close(s);
  };

  auto on_error = [](auto err) {
    std::cout << "Error connecting: " << err << std::endl;
  };

  // let's try to connect
  connect_to((args.size() > 1) ? args[1] : "localhost",
             (args.size() > 2) ? args[2] : "9921", on_connected, on_error);
  return 0;
}
