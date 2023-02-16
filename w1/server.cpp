#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include "socket_tools.h"
#include "packet.h"

int main(int argc, const char **argv)
{
  const char *portListen = "2022";
  const char *portSend = "2023";

  int sfdListen = create_dgram_socket(nullptr, portListen, nullptr);

  if (sfdListen == -1)
    return 1;
  printf("listening!\n");

  addrinfo resAddrInfo;
  int sfdSend;

  while (true)
  {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sfdListen, &readSet);

    timeval timeout = { 0, 100000 }; // 100 ms
    select(sfdListen + 1, &readSet, NULL, NULL, &timeout);


    if (FD_ISSET(sfdListen, &readSet))
    {
      Packet packet = get_packet(sfdListen);
      switch (packet.packetType)
      {
        case INIT:
          std::cout << "Client" << packet.clientId << " connected\n";
          sfdSend = create_dgram_socket("localhost", portSend, &resAddrInfo);
          send_packet(sfdSend, resAddrInfo, &packet);
          break;
        case DATA:
          std::cout << "Received message from Client" << packet.clientId << ": " << packet.message << "\n";
          packet.message *= 2;
          send_packet(sfdSend, resAddrInfo, &packet);
          break;
        case KEEP_ALIVE:
          break;
      }
    }
  }
  return 0;
}
