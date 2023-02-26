#include <enet/enet.h>

#include <cstring>
#include <iostream>
#include <thread>

#include "util.h"

void process_input(ENetPeer *lobbyPeer, bool connected)
{
  while (!connected)
  {
    std::string inputString;
    std::getline(std::cin, inputString);
    send_rel_micro_packet(lobbyPeer, inputString.c_str());
  }
}

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }

  ENetHost *client = enet_host_create(nullptr, 2, 2, 0, 0);
  if (!client)
  {
    printf("Cannot create ENet client\n");
    return 1;
  }

  ENetAddress lobbyAddress;
  enet_address_set_host(&lobbyAddress, "localhost");
  lobbyAddress.port = 10887;

  ENetPeer *serverPeer = nullptr;
  ENetPeer *lobbyPeer = enet_host_connect(client, &lobbyAddress, 2, 0);
  if (!lobbyPeer)
  {
    printf("Cannot connect to lobby");
    return 1;
  }

  bool connected = false;

  std::thread input_thread(process_input, lobbyPeer, connected);

  uint32_t timeStart = enet_time_get();
  uint32_t lastMicroSendTime = timeStart;
  while (true)
  {
    ENetEvent event;
    while (enet_host_service(client, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        break;
      case ENET_EVENT_TYPE_RECEIVE:
      {
        std::string dataString{(const char *)event.packet->data};
        std::string packetType{dataString.substr(0, 4)};
        if (packetType.compare("port") == 0)
        {
          ENetAddress serverAddress;
          enet_address_set_host(&serverAddress, "localhost");
          serverAddress.port = std::atoi(dataString.c_str() + 4);
          printf("Connecting to %x:%u...\n", serverAddress.host, serverAddress.port);
          serverPeer = enet_host_connect(client, &serverAddress, 2, 0);
          if (!serverPeer)
          {
            printf("Can't connect to %x:%u...\n", serverAddress.host, serverAddress.port);
            return 1;
          }
          connected = true;
        }
        else if (packetType.compare("time") == 0)
        {
          printf("Server time: %s\n", dataString.c_str() + 4);
        }
        else if (packetType.compare("ping") == 0)
        {
          printf("Player pings:\n%s", dataString.c_str() + 4);
        }
        else if (packetType.compare("list") == 0)
        {
          printf("Player list: %s\n", dataString.c_str() + 4);
        }
        else
        {
        printf("Packet received '%s'\n", dataString.c_str());
        }
        enet_packet_destroy(event.packet);
        break;
      }
      default:
        break;
      };
    }
    if (connected)
    {
      uint32_t curTime = enet_time_get();
      if (curTime - lastMicroSendTime > 5000)
      {
        lastMicroSendTime = curTime;
        send_rel_micro_packet(serverPeer, get_time_message().c_str());
      }
    }
  }

  input_thread.join();

  return 0;
}
