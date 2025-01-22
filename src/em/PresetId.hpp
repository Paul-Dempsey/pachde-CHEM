#pragma once
#include <rack.hpp>
#include "../services/FixedStringBuffer.hpp"

namespace pachde {

#pragma pack(push, 1)
typedef union RawPresetId {
    uint32_t data;
    struct {
        uint8_t number;  // program change
        uint8_t bank_lo; // cc32
        uint8_t bank_hi; // cc0
        uint8_t pad;
    } bytes;
} RawPresetId;
#pragma pack(pop)


class PresetId {
    RawPresetId id{0};

public:
    PresetId(){ id.data = 0; }
    PresetId(const PresetId& other) { id = other.id; }
    PresetId(uint32_t key) { id.data = key; }
    PresetId(uint8_t hi, uint8_t lo, uint8_t number) {
        id.bytes.bank_hi = hi;
        id.bytes.bank_lo = lo;
        id.bytes.number = number;
        id.bytes.pad = 0;
    }
    RawPresetId& raw() { return id; }
    uint32_t key() { return id.data; }
    uint8_t bankHi() { return id.bytes.bank_hi; }
    uint8_t bankLo() { return id.bytes.bank_lo; }
    uint8_t number() { return id.bytes.number; }
};

}