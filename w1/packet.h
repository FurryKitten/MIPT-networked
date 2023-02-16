#pragma once

#include <netdb.h>
#include <iostream>
#include <string>
#include <cstring>

#include "socket_tools.h"

enum PACKET_TYPE : uint32_t
{
    INIT,
    DATA,
    KEEP_ALIVE
};

struct Packet
{
  PACKET_TYPE packetType;
  uint32_t clientId;
  uint32_t message;
};

ssize_t send_packet(int sfd, addrinfo addrInfo, Packet *packet) {
    size_t packetSize = sizeof(Packet);
    if (packet->packetType == INIT)
        packetSize = sizeof(uint32_t) * 2;
    else if (packet->packetType == KEEP_ALIVE)
        packetSize = sizeof(uint32_t);

    ssize_t res = sendto(sfd, packet, packetSize, 0, addrInfo.ai_addr, addrInfo.ai_addrlen);
    if (res == -1)
      std::cout << strerror(errno) << std::endl;
    return res;
}

Packet get_packet(int sfd)
{
    Packet packet;
    ssize_t numBytes = recvfrom(sfd, &packet, sizeof(Packet), 0, nullptr, nullptr);
    return packet;
}

/* 
g++ client.cpp socket_tools.cpp -std=c++17 -o client && ./client
g++ server.cpp socket_tools.cpp -std=c++17 -o server && ./server
 */