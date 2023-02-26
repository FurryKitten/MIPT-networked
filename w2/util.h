#pragma once

#include <enet/enet.h>

#include <cstring>
#include <ctime>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

#define REL_CHANNEL 0
#define UNREL_CHANNEL 1

struct Player
{
  uint32_t id;
  std::string name;
};

void send_rel_micro_packet(ENetPeer *peer, const char *msg)
{
  ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, REL_CHANNEL, packet);
}

void send_unrel_micro_packet(ENetPeer *peer, const char *msg)
{
  ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_UNSEQUENCED);
  enet_peer_send(peer, UNREL_CHANNEL, packet);
}

void send_rel_to_all(std::map<ENetPeer *, Player> &peers, std::string message)
{
  for (const auto &peer : peers)
  {
    send_rel_micro_packet(peer.first, message.c_str());
  }
}

void send_unrel_to_all(std::map<ENetPeer *, Player> &peers, std::string message)
{
  for (const auto &peer : peers)
  {
    send_unrel_micro_packet(peer.first, message.c_str());
  }
}

std::string get_player_list_message(std::map<ENetPeer *, Player> &peers)
{
  std::stringstream msg;
  msg << "list";
  for (const auto &peer : peers)
  {
    msg << peer.second.name << ", ";
  }
  return msg.str().erase(msg.str().size() - 2);
}

std::string get_ping_message(std::map<ENetPeer *, Player> &peers)
{
  std::stringstream msg;
  msg << "ping";
  for (const auto &peer : peers)
  {
    msg << "\t" << peer.second.name << " : " << peer.first->roundTripTime << " ms\n";
  }
  return msg.str();
}

std::string get_time_message()
{
  srand(std::time(nullptr));
  auto t = std::time(nullptr) - rand() % 1000;
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << "time" << std::put_time(&tm, "%d.%m.%Y %H:%M:%S") << "\n";
  return oss.str();
}