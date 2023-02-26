#include <enet/enet.h>
#include <iostream>
#include <set>
#include "util.h"

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10887;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  bool started = false;
  std::string serverPortMessage = "port10889";
  std::set<ENetPeer *> peers;

  while (true)
  {
    ENetEvent event;
    while (enet_host_service(server, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        peers.insert(event.peer);
        if (started)
          send_rel_micro_packet(event.peer, serverPortMessage.c_str());
        break;
      case ENET_EVENT_TYPE_RECEIVE:
      {
        std::string dataString{(const char *)event.packet->data};
        printf("Packet received '%s'\n", dataString.c_str());
        if (dataString.compare("start") == 0)
        {
          for (ENetPeer *peer : peers)
          {
            send_rel_micro_packet(peer, serverPortMessage.c_str());
          }
          started = true;
        }
        enet_packet_destroy(event.packet);
        break;
      }
      case ENET_EVENT_TYPE_DISCONNECT:
        printf("Disconnected %x:%u\n", event.peer->address.host, event.peer->address.port);
        peers.erase(event.peer);
        break;
      default:
        break;
      };
    }
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}
