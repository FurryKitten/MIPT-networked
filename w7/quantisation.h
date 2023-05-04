#pragma once

#include "mathUtils.h"
#include <limits>
#include <iostream>

template<typename T>
T pack_float(float v, float lo, float hi, int num_bits)
{
  T range = (1 << num_bits) - 1;//std::numeric_limits<T>::max();
  return range * ((clamp(v, lo, hi) - lo) / (hi - lo));
}

template<typename T>
float unpack_float(T c, float lo, float hi, int num_bits)
{
  T range = (1 << num_bits) - 1;//std::numeric_limits<T>::max();
  return float(c) / range * (hi - lo) + lo;
}

template<typename T, int num_bits>
struct PackedFloat
{
  T packedVal;

  PackedFloat(float v, float lo, float hi) { pack(v, lo, hi); }
  PackedFloat(T compressed_val) : packedVal(compressed_val) {}

  void pack(float v, float lo, float hi) { packedVal = pack_float<T>(v, lo, hi, num_bits); }
  float unpack(float lo, float hi) { return unpack_float<T>(packedVal, lo, hi, num_bits); }
};

typedef PackedFloat<uint8_t, 4> float4bitsQuantized;

struct Vec2{ float x, y; };
struct Vec3{ float x, y, z; };
std::ostream &operator<<(std::ostream &os, Vec2 const &v);
std::ostream &operator<<(std::ostream &os, Vec3 const &v);

template<typename T, int num_bits1, int num_bits2>
struct PackedVec2 {
    T packedVal;

    PackedVec2(float x, float y, float lo, float hi) { pack(Vec2{x, y}, lo, hi); }
    PackedVec2(Vec2 vec, float lo, float hi) { pack(vec, lo, hi); }
    PackedVec2(T compressed_val) : packedVal(compressed_val) {}

    void pack(Vec2 vec, float lo, float hi)
    {
        T packed1 = pack_float<T>(vec.x, lo, hi, num_bits1);
        T packed2 = pack_float<T>(vec.y, lo, hi, num_bits2);
        packedVal = packed1 << num_bits2 | packed2;
    }

    Vec2 unpack(float lo, float hi)
    {
        Vec2 unpacked{};
        unpacked.x = unpack_float<T>(packedVal >> num_bits2, lo, hi, num_bits1);
        unpacked.y = unpack_float<T>(packedVal & ((1 << num_bits2) - 1), lo, hi, num_bits2);
        return unpacked;
    }
};


template<typename T, int num_bits1, int num_bits2, int num_bits3>
struct PackedVec3 {
    T packedVal;

    PackedVec3(float x, float y, float z, float lo, float hi) { pack(Vec3{x, y, z}, lo, hi); }
    PackedVec3(Vec3 vec, float lo, float hi) { pack(vec, lo, hi); }
    PackedVec3(T compressed_val) : packedVal(compressed_val) {}

    void pack(Vec3 vec, float lo, float hi)
    {
        T packed1 = pack_float<T>(vec.x, lo, hi, num_bits1);
        T packed2 = pack_float<T>(vec.y, lo, hi, num_bits2);
        T packed3 = pack_float<T>(vec.z, lo, hi, num_bits3);
        packedVal = packed1 << (num_bits2 + num_bits3) | packed2 << num_bits3 | packed3;
    }

    Vec3 unpack(float lo, float hi)
    {
        Vec3 unpacked{};
        unpacked.x = unpack_float<T>(packedVal >> (num_bits2 + num_bits3), lo, hi, num_bits1);
        unpacked.y = unpack_float<T>((packedVal >> num_bits3) & ((1 << num_bits2) - 1), lo, hi, num_bits2);
        unpacked.z = unpack_float<T>(packedVal & ((1 << num_bits3) - 1), lo, hi, num_bits3);
        return unpacked;
    }
};

void* pack_uint32(uint32_t val);
uint32_t unpack_uint32(void* ptr);

bool isQuantizedRight(float defaultVal, float packedVal, float lo, float hi, int num_bits);
void test();