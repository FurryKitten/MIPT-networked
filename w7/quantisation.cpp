#include "quantisation.h"

inline std::ostream &operator<<(std::ostream &os, Vec2 const &v) {
    return os << "{" << v.x << ", " << v.y << "}";
}
inline std::ostream &operator<<(std::ostream &os, Vec3 const &v) {
    return os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
}

const uint32_t limit_8bit = std::numeric_limits<uint8_t>::max() / 2;
const uint32_t limit_16bit = std::numeric_limits<uint16_t>::max() / 4;
const uint32_t limit_32bit = std::numeric_limits<uint32_t>::max() / 4;

void *pack_uint32(uint32_t val) {
    if (val < limit_8bit)
    {
        uint8_t* ptr = new uint8_t(0);
        *ptr = val << 1;
        return ptr;
    }
    else if (val < limit_16bit)
    {
        uint16_t* ptr = new uint16_t(0);
        *ptr = (val << 2) + 1;
        return ptr;
    }
    else if (val < limit_32bit)
    {
        uint32_t* ptr = new uint32_t(0);
        *ptr = (val << 2) + 3;
        return ptr;
    }
    return nullptr;
}

uint32_t unpack_uint32(void *ptr) {
    uint32_t unpackedVal = 0;
    uint8_t value_uint8 = *(uint8_t*) ptr;
    uint16_t value_uint16 = *(uint16_t*) ptr;
    uint32_t value_uint32 = *(uint32_t*) ptr;

    if ((~value_uint32 & 0x3) == 0)
    {
        unpackedVal = value_uint32 >> 2;
    }
    else if ((value_uint16 & 0x2) == 0)
    {
        unpackedVal = value_uint16 >> 2;
    }
    else if ((value_uint8 & 0x1) == 0)
    {
        unpackedVal = value_uint8 >> 1;
    }

    return unpackedVal;
}

bool isQuantizedRight(float defaultVal, float packedVal, float lo, float hi, int num_bits)
{
    int range = (1 << num_bits) - 1;
    return abs(defaultVal - packedVal) < ((hi - lo) / range);
}

void test()
{
    Vec2 vec2{2.3f, 4.0f};
    Vec3 vec3{10.0f, 200.0f, -40.0f};

    PackedVec2<uint32_t, 16, 16> packedVec2{vec2, -5, 5};
    PackedVec3<uint64_t, 20, 24, 20> packedVec3{vec3, -500, 500};

    std::cout << "Vec2: " << vec2 << "\nUnpacked Vec2: " << packedVec2.unpack(-5, 5) << "\n";
    std::cout << std::boolalpha << "QuantizedRight: "
              << isQuantizedRight(vec2.x, packedVec2.unpack(-5, 5).x, -5, 5, 16) << "\n\n";

    std::cout << "Vec3: " << vec3 << "\nUnpacked Vec3: " << packedVec3.unpack(-500, 500) << "\n";
    std::cout << std::boolalpha << "QuantizedRight: "
              << isQuantizedRight(vec3.x, packedVec3.unpack(-500, 500).x, -500, 500, 20) << "\n\n";

    void* packed8 = pack_uint32(123);
    void* packed16 = pack_uint32(1043);
    void* packed32 = pack_uint32(32000);

    uint32_t unpacked8 = unpack_uint32(packed8);
    uint32_t unpacked16 = unpack_uint32(packed16);
    uint32_t unpacked32 = unpack_uint32(packed32);

    std::cout << "Packing to uint8:  123;   Unpacking: " << unpacked8 << "\n";
    std::cout << "Packing to uint16: 1043;  Unpacking: " << unpacked16 << "\n";
    std::cout << "Packing to uint32: 32000; Unpacking: " << unpacked32 << "\n";
}



