#pragma once
#include <rack.hpp>
#include "em/midi-message.h"
#include "em/wrap-HakenMidi.hpp"

namespace pachde {

typedef union {
    uint32_t u;
    char name[4];
} OC;

enum Reserved: uint32_t {
    Macro         = 0x0000006d, // "m"
    Stream        = 0x00000073, // "s"
    Cc            = 0x00006363, // "cc"
    ProgramChange = 0x00006370, // "pc"
    Channel       = 0x00006863, // "ch"
    BiqSin_poke   = 0x00007062, // "bp"
    Formula_poke  = 0x00007066, // "fp"
    Graph_poke    = 0x00007067, // "gp"
    Kinetic_poke  = 0x0000706b, // "kp"
    Matrix_poke   = 0x0000706d, // "mp"
    Conv_poke     = 0x00007076, // "vp"
};

struct Opcode {
    OC code{0};
    Opcode() {}
    Opcode(char a, char b) {code.name[0] = a; code.name[1] = b; }
    Opcode(char a, char b, char c) { code.name[0] = a; code.name[1] = b; code.name[2] = c; }
    char * str () { return &code.name[0]; }
    uint32_t nom() { return code.u; }
};


enum class NumberSize
{
    // trick: Values are max value for the type
    SevenBit = 127,
    FourteenBit = Haken::max14
};

struct HclCompiler
{
    bool ok{true};
    int error_pos{0};
    std::string error_message;
    const char * program_start;

    uint8_t channel{0};
    uint16_t value;
    Opcode code;
    std::map<std::string, uint16_t> variables;

    void error(const std::string& error, const char * pos);

    std::vector<PackedMidiMessage>* dest{nullptr};
    bool compile(const std::string& program, std::vector<PackedMidiMessage>* midi);
    const char * parse_opcode(const char *scan);
    const char * scan_opcode(const char *scan);
    const char * scan_variable_def(const char *scan);
    const char * scan_var_number(const char *scan, NumberSize size);
    const char * scan_rational(const char *scan);
    const char * scan_number(const char *scan, NumberSize size);
    const char * scan_channel(const char *scan);
    const char * scan_cc(const char *scan);
    const char * scan_pc(const char *scan);
    const char * scan_macro(const char *scan);
    const char * scan_stream(const char *scan);
    const char * scan_graph(const char *scan);
    const char * scan_poke(const char * scan, uint8_t poke_stream);
    const char * scan_list(const char *scan);
};

}