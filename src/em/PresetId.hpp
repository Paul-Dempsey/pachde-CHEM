#pragma once

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


class PresetId
{
private:
    RawPresetId id;
    void make_valid() { id.bytes.pad = 0; }
public:
    static const uint32_t InvalidKey = 0xff000000;
    PresetId(){ invalidate(); }
    PresetId& operator=(const PresetId& other) { id = other.id; return *this; }
    bool valid() { return 0 == id.bytes.pad; }
    void invalidate() { id.data = InvalidKey; }
    bool operator==(const PresetId& other) { return id.data == other.id.data; }
    PresetId(const PresetId& other) { id = other.id; }
    PresetId(uint32_t key) { id.data = key; }
    PresetId(uint8_t hi, uint8_t lo, uint8_t number) {
        id.bytes.bank_hi = hi;
        id.bytes.bank_lo = lo;
        id.bytes.number = number;
        id.bytes.pad = 0;
    }
    void set_bank_hi(uint8_t hi) { id.bytes.bank_hi = hi; make_valid(); }
    void set_bank_lo(uint8_t low) { id.bytes.bank_lo = low; make_valid(); }
    void set_number(uint8_t n) { id.bytes.number = n; make_valid(); }
    void clear() { invalidate(); }
    //RawPresetId& raw() { return id; }
    uint32_t key() const { return id.data; }
    uint8_t bank_hi() const { return id.bytes.bank_hi; }
    uint8_t bank_lo() const { return id.bytes.bank_lo; }
    uint8_t number() const { return id.bytes.number; }
};

}