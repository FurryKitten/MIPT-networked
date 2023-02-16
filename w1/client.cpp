#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <chrono>
#include <thread>
#include "socket_tools.h"
#include "packet.h"

void init_connection(int sfd, addrinfo resAddrInfo, uint32_t clientId)
{
  Packet packet{INIT, clientId};
  ssize_t res = send_packet(sfd, resAddrInfo, &packet);
  if (res != -1)
  {
    std::cout << "Connected to server with client ID " << clientId << "!\n";
  }
}

void listen_to_server(int sfd, const char *portListen)
{
  int sfdListen = create_dgram_socket(nullptr, portListen, nullptr);
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
          std::cout << "Welcome to server, Client" << packet.clientId << "\n";
          break;
        case DATA:
          std::cout << "Received message from Server: " << packet.message << "\n";
          break;
      }
    }
  }
}

void keep_alive(int sfd, addrinfo addrInfo, uint32_t clientId)
{
  Packet packet{KEEP_ALIVE, clientId, 0};
  while (true)
  {
    ssize_t res = send_packet(sfd, addrInfo, &packet);
    if (res != -1)
    {
      std::cout << "Sent keep_alive packet\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
}

int main(int argc, const char **argv)
{
  srand(time(0));
  uint32_t clientId = rand();

  const char *portSend = "2022";
  const char *portListen = "2023";

  addrinfo resAddrInfo;
  int sfd = create_dgram_socket("localhost", portSend, &resAddrInfo);

  if (sfd == -1)
  {
    printf("Cannot create a socket\n");
    return 1;
  }

  std::thread keepAliveThread(keep_alive, sfd, resAddrInfo, clientId);
  std::thread listenToServerThread(listen_to_server, sfd, portListen);
  keepAliveThread.detach();
  listenToServerThread.detach();

  init_connection(sfd, resAddrInfo, clientId);

  Packet packet{DATA, clientId, 0};
  while (true)
  {
    ssize_t res = send_packet(sfd, resAddrInfo, &packet);
    if (res != -1)
    {
      std::cout << "Sent message to Server: " << packet.message << "\n";
      packet.message++;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return 0;
}
