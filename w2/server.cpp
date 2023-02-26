#include <enet/enet.h>

#include <iostream>
#include <map>

#include "util.h"

std::string get_player_name()
{
  return "Player" + std::to_string(rand() % 10000);
}

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }

  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10889;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  std::map<ENetPeer *, Player> peers;
  uint32_t playerCounter = 0;

  uint32_t timeStart = enet_time_get();
  uint32_t lastTimeSendTime = timeStart;
  uint32_t lastPingSendTime = timeStart;

  while (true)
  {
    ENetEvent event;
    while (enet_host_service(server, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        peers[event.peer] = Player{playerCounter++, get_player_name()};
        printf("Connection with %s established\n", peers[event.peer].name.c_str());
        send_rel_to_all(peers, get_player_list_message(peers));
        break;
      case ENET_EVENT_TYPE_RECEIVE:
      {
        std::string dataString{(const char *)event.packet->data};
        if (dataString.substr(0, 4).compare("time") == 0)
        {
          printf("%s current time is: %s", peers[event.peer].name.c_str(), dataString.c_str() + 4);
        }
        enet_packet_destroy(event.packet);
        break;
      }
      case ENET_EVENT_TYPE_DISCONNECT:
        printf("Disconnected %s\n", peers[event.peer].name.c_str());
        peers.erase(event.peer);
        break;
      default:
        break;
      };
    }

    if (peers.empty())
      continue;

    uint32_t curTime = enet_time_get();
    if (curTime - lastTimeSendTime > 5000)
    {
      lastTimeSendTime = curTime;
      send_rel_to_all(peers, get_time_message());
    }
    if (curTime - lastPingSendTime > 2000)
    {
      lastPingSendTime = curTime;
      send_unrel_to_all(peers, get_ping_message(peers));
    }
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}
