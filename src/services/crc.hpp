#pragma once
#include <stdint.h>

namespace crc {
    typedef uint32_t(CRC32TABLE)[256];

    static void init_crc32(CRC32TABLE& tab)
    {
        uint32_t polynomial = 0xedb88320;
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t n = i;
            for (size_t j = 0; j < 8; ++j) {
                n = (n & 1) ? (polynomial ^ (n >> 1)) : n >> 1;
            }
            tab[i] = n;
        }
    }

    static uint32_t accumulate(CRC32TABLE& tab, uint32_t crc_seed, const void * data, size_t byte_count)
    {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        uint32_t n = crc_seed ^ 0xffffffff;
        for (size_t i = 0; i < byte_count; ++i) {
            n = tab[(n ^ *bytes++) & 0xff] ^ (n >> 8);
        }
        return n ^ 0xffffffff;
    }

    class crc32
    {
    private:
        CRC32TABLE lut;
        uint32_t sum{0};
        size_t total{0};
    public:
        crc32() {
            init_crc32(lut);
        }
        void init() { sum = 0; total = 0; }
        uint32_t result() { return sum; }
        uint32_t content_size() { return total; }
        uint32_t accumulate(const void * data, size_t byte_count)
        {
            total += byte_count;
            sum = crc::accumulate(lut, sum, data, byte_count);
            return sum;
        }
    };
}

