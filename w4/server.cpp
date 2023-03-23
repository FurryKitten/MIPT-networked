#include <enet/enet.h>
#include <iostream>
#include "entity.h"
#include "raymath.h"
#include "protocol.h"
#include <stdlib.h>
#include <vector>
#include <map>

static std::vector<Entity> entities;
static std::map<uint16_t, ENetPeer*> controlledMap;
static std::map<uint16_t, Vector2> target_positions_by_id;

const uint32_t MAX_ENTITIES = 30;
const float ENTITY_MAX_RADIUS = 20.f;
const float ENTITY_MIN_RADIUS = 5.f;
const float AI_SPEED = 50.f;

Entity create_entity(EntityOwner owner)
{
  uint16_t maxEid = entities.empty() ? invalid_entity : entities[0].eid;
  for (const Entity &e : entities)
      maxEid = std::max(maxEid, e.eid);
  uint16_t newEid = maxEid + 1;
  float radius = std::clamp((((float) rand()) / (float) RAND_MAX) * 20.f, ENTITY_MIN_RADIUS, ENTITY_MAX_RADIUS);
  uint32_t color = 0x000000ff +
                   0x00440000 * (rand() % 5) +
                   0x00004400 * (rand() % 5) +
                   0x44000000 * (rand() % 5);
  Vector2 pos {(rand() % 100) * 6.f - 300.f, (rand() % 100) * 6.f - 300.f};
  Entity ent = {pos, radius, GetColor(color), newEid, owner};
  return ent;
}

void on_join(ENetPacket *packet, ENetPeer *peer, ENetHost *host)
{
  // send all entities
  for (const Entity &ent : entities)
    send_new_entity(peer, ent);

  // find max eid
  Entity ent = create_entity(EntityOwner::CLIENT_CONTROLLED);
  ent.radius = 12.f;
  entities.push_back(ent);

  controlledMap[ent.eid] = peer;


  // send info about new entity to everyone
  for (size_t i = 0; i < host->peerCount; ++i)
    send_new_entity(&host->peers[i], ent);
  // send info about controlled entity
  send_set_controlled_entity(peer, ent.eid);
}

void on_state(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  Vector2 pos;
  float radius;
  deserialize_entity_state(packet, eid, pos, radius);
  for (Entity &e : entities)
    if (e.eid == eid)
    {
      e.pos = pos;
      e.radius = radius;
    }
}

void create_ai_entities(int max_entities, ENetHost *host)
{
  for (int i = 0; i < max_entities - entities.size(); ++i)
  {
    Entity ent = create_entity(EntityOwner::SERVER_CONTROLLED);
    entities.push_back(ent);
    target_positions_by_id[ent.eid] = Vector2({(rand() % 100) * 6.f - 300.f, (rand() % 100) * 6.f - 300.f});

    // send info about new entity to everyone
    for (size_t i = 0; i < host->peerCount; ++i)
      send_new_entity(&host->peers[i], ent);
  }
}

void update_ai_entity(Entity &ent)
{
  float dt = 1 / 60.f;
  Vector2 &pos = target_positions_by_id[ent.eid];
  if (Vector2DistanceSqr(ent.pos, pos) < ent.radius * ent.radius)
  {
    pos = Vector2({(rand() % 100) * 6.f - 300.f, (rand() % 100) * 6.f - 300.f});
  }
  Vector2 dir = Vector2Normalize(Vector2Subtract(pos, ent.pos));
  ent.pos = Vector2Add(ent.pos, Vector2Scale(dir, dt * AI_SPEED));
}

void update_collisions(Entity &ent)
{
  for (Entity &e : entities)
  {
    if (ent.eid == e.eid)
      continue;
    if (Vector2DistanceSqr(ent.pos, e.pos) > (ent.radius + e.radius) * (ent.radius + e.radius))
      continue;
    if (ent.radius > e.radius)
    {
      ent.radius = std::clamp(ent.radius + e.radius / 2.f, ENTITY_MIN_RADIUS, ENTITY_MAX_RADIUS);
      e.radius = std::clamp(e.radius / 2.f, ENTITY_MIN_RADIUS, ENTITY_MAX_RADIUS);
      e.pos = {(rand() % 100) * 6.f - 300.f, (rand() % 100) * 6.f - 300.f};
      if (e.owner == CLIENT_CONTROLLED)
        send_snapshot(controlledMap[e.eid], e.eid, e.pos, e.radius);
      if (ent.owner == CLIENT_CONTROLLED)
        send_snapshot(controlledMap[ent.eid], ent.eid, ent.pos, ent.radius);
    }
  }
}

int main(int argc, const char **argv)
{
  srand(time( 0 ));
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10131;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  create_ai_entities(MAX_ENTITIES, server);

  while (true)
  {
    ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        switch (get_packet_type(event.packet))
        {
          case E_CLIENT_TO_SERVER_JOIN:
            on_join(event.packet, event.peer, server);
            break;
          case E_CLIENT_TO_SERVER_STATE:
            on_state(event.packet);
            break;
        };
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
    static int t = 0;
    for (Entity &e : entities)
    {
      if (e.owner == SERVER_CONTROLLED)
      {
        update_ai_entity(e);
      }
      update_collisions(e);
      for (size_t i = 0; i < server->peerCount; ++i)
      {
        ENetPeer *peer = &server->peers[i];
        if (e.owner == EntityOwner::SERVER_CONTROLLED || controlledMap[e.eid] != peer)
          send_snapshot(peer, e.eid, e.pos, e.radius);
      }
    }
    usleep(1000000 / 60);
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}


