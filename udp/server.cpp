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
 * @brief this is shorter version
 */
int bind_socket(std::function<void(int)> success, unsigned int port_nr = 9922) {
  int s = socket(AF_INET6, SOCK_DGRAM, 0);
  if (s < 0)
    throw std::system_error(errno, std::generic_category());

  if (const int off = 0;
      setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off))) {
    close(s);
    throw std::system_error(errno, std::generic_category());
  }

  struct sockaddr_in6 addr {};
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port_nr);
  socklen_t alen = sizeof(addr);
  if (bind(s, (struct sockaddr *)(&addr), alen)) {
    close(s);
    throw std::system_error(errno, std::generic_category());
  }

  success(s);
  return s;
}

int main(int argc, char **argv) {
  auto socket_ready = [&](int udp_socket) {
    std::cout << "(server) Waiting for connection..." << std::endl;
    char buffer[128];
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
    while (1) {
      ssize_t rsize = recvfrom(udp_socket, buffer, 128, MSG_DONTWAIT,
                               (struct sockaddr *)&peer_addr, &peer_addr_len);
      if (rsize > 0) {
        std::cout << "(server) Got: " << buffer << std::endl;
        if (rsize > 0) {
          char host[NI_MAXHOST], service[NI_MAXSERV];
          getnameinfo((struct sockaddr *)&peer_addr, peer_addr_len, host,
                      NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
          std::cout << "(server)    from: " << host << " : " << service
                    << std::endl;
          sendto(udp_socket, buffer, 128, 0, (struct sockaddr *)&peer_addr,
                 peer_addr_len);
        }
        break;
      }
      std::cout << "waiting..." << std::endl;
      sleep(1);
    }
    ::close(udp_socket);
  };

  bind_socket(socket_ready, 9921);
  return 0;
}
