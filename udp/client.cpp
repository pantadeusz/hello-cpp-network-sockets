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

// helper class with some c++ magic
// it wraps sockaddr in a structure that handles memory
class net_addr {
public:
  std::shared_ptr<sockaddr> addr_ptr;
  const sockaddr *addr;
  const socklen_t addrlen;
  net_addr(const sockaddr *a, const socklen_t l)
      : addr_ptr(
            [&]() {
              sockaddr *p = (sockaddr *)new char[l];
              std::copy((char *)a, (char *)a + l, (char *)p);
              return p;
            }(),
            [](sockaddr *p) { delete[](char *) p; }),
        addr(addr_ptr.get()), addrlen(l){};
};

/**
 * @brief finds server for the given host, port and address family
 *
 * @return list of found addresses
 */
std::vector<net_addr> find_addresses(const char *addr_txt, const char *port_txt,
                                     int ai_family = AF_UNSPEC) {

  struct addrinfo hints{};
  //std::fill((char *)&hints, (char *)&hints + sizeof(struct addrinfo), 0);
  hints.ai_family = ai_family;    ///< IPv4 or IPv6
  hints.ai_socktype = SOCK_DGRAM; ///< datagram socket

  struct addrinfo *addr_p,*rp;
  if (int err = getaddrinfo(addr_txt, port_txt, &hints, &addr_p); err) {
      throw std::system_error(errno, std::generic_category());
  }
  std::vector<net_addr> ret;
  for (rp = addr_p; rp != NULL; rp = rp->ai_next) ret.push_back({rp->ai_addr, rp->ai_addrlen});  
  freeaddrinfo(addr_p);
  return ret;
}

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

/**
 * @brief The main function.
 *
 * it binds local address
 */

int main(int argc, char **argv) {
  auto on_bind = [&](int s) {
    auto addrs = find_addresses("localhost", "9921");

    std::string msg = "Hi! How are you?";
    std::vector<char> msg_to_send(msg.begin(), msg.end());
    msg_to_send.resize(128);

    if (sendto(s, msg_to_send.data(), 128, 0, addrs.at(0).addr,
               addrs.at(0).addrlen) == -1) {
      throw std::system_error(errno, std::generic_category());
    } else {
      struct sockaddr_storage peer_addr;
      socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
      ssize_t rsize = recvfrom(s, msg_to_send.data(), 128, 0,
                               (struct sockaddr *)&peer_addr, &peer_addr_len);
      std::cout << "(client) got pinged message: " << msg_to_send.data()
                << std::endl;
    }
    ::close(s);
  };
  bind_socket(on_bind, 9929);
  return 0;
}
