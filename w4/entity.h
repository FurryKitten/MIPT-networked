#pragma once
#include "raylib.h"
#include <cstdint>

constexpr uint16_t invalid_entity = -1;

enum EntityOwner
{
  NONE,
  SERVER_CONTROLLED,
  CLIENT_CONTROLLED
};

struct Entity
{
  Vector2 pos {0, 0};
  float radius = 10.0f;
  Color color = BLACK;
  uint16_t eid = invalid_entity;
  EntityOwner owner = NONE;
};

