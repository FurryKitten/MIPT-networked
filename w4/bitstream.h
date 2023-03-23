#pragma once

#include <cstdint>
#include <cstring>

class Bitstream {
public:
    Bitstream(uint8_t *ptr) : ptr_(ptr) {}

    template<typename T>
    void write(const T &val) {
        memcpy(ptr_, (const uint8_t*) &val, sizeof(T));
        ptr_ += sizeof(T);
    }

    template<typename T>
    void read(T &val) {
        memcpy((uint8_t*) &val, ptr_, sizeof(T));
        ptr_ += sizeof(T);
    }

private:
    uint8_t *ptr_;
};
