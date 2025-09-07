#include "hcl.hpp"
#include "../../chem-id.hpp"
#include "../../services/misc.hpp"

namespace pachde {
/*
            comment : `"` nonquote* `"`
           variable : `{` name `=` value `}`

     channel-number : 1 .. 16
                 n7 : 0 .. 127
                n14 : 0 .. 16256
      stream-number : 0 .. 27
       macro-number : 1 .. 90
           cc-value : n7 | n14
        macro-value : n7 | n14 | signed-rational
             digits : 0 .. 9 | '_'
    signed-rational : `-1` | +1 | `zero` | [`+`|`-`](`.`|`,`)digits
               pair : n7 n7
          pair-list : '[' pair+ ']'
               list : '[' n7+ ']'

            channel : `ch` channel-number
       control-code : `cc` n7 cc-value
     program-change : 'pc' n7
              macro : `m` macro-number macro-value
             stream : `s` stream-number list
        matrix-poke : `mp` pair-list
       formula-poke : `fp` pair-list
         graph-poke : (`gp`|`gp1`|`gp2`) pair-list
         kinet-poke : `kp` pair-list
         bqsin-poke : `bp` pair-list
          conv-poke : `vp` pair-list
*/
#define TABLE_BIT(m) (uint64_t(1) << uint64_t(m))
constexpr const uint64_t LO_14BIT_CC =
    TABLE_BIT(Haken::ccMod) |
    TABLE_BIT(Haken::ccBreath) |
    TABLE_BIT(Haken::ccUndef) |
    TABLE_BIT(Haken::ccFoot) |
    TABLE_BIT(Haken::ccVol) |
    TABLE_BIT(Haken::ccExpres) |
    TABLE_BIT(Haken::ccI) |
    TABLE_BIT(Haken::ccII) |
    TABLE_BIT(Haken::ccIII) |
    TABLE_BIT(Haken::ccIV) |
    TABLE_BIT(Haken::ccV) |
    TABLE_BIT(Haken::ccVI) |
    TABLE_BIT(Haken::ccPost) |
    TABLE_BIT(Haken::ccReci1) |
    TABLE_BIT(Haken::ccReci2) |
    TABLE_BIT(Haken::ccReci3) |
    TABLE_BIT(Haken::ccReci4) |
    TABLE_BIT(Haken::ccReciMix) |
    TABLE_BIT(Haken::ccJack1) |
    TABLE_BIT(Haken::ccJack2) |
    TABLE_BIT(Haken::ccM7) |
    TABLE_BIT(Haken::ccM8) |
    TABLE_BIT(Haken::ccM9) |
    TABLE_BIT(Haken::ccM10) |
    TABLE_BIT(Haken::ccM11) |
    TABLE_BIT(Haken::ccM12) |
    TABLE_BIT(Haken::ccM13) |
    TABLE_BIT(Haken::ccM14) |
    TABLE_BIT(Haken::ccM15) |
    TABLE_BIT(Haken::ccM16) |
    TABLE_BIT(Haken::ccM17) |
    TABLE_BIT(Haken::ccM18) |
    TABLE_BIT(Haken::ccM19) |
    TABLE_BIT(Haken::ccM20) |
    TABLE_BIT(Haken::ccM21) |
    TABLE_BIT(Haken::ccM22) |
    TABLE_BIT(Haken::ccM23) |
    TABLE_BIT(Haken::ccM24) |
    TABLE_BIT(Haken::ccM25) |
    TABLE_BIT(Haken::ccM26) |
    TABLE_BIT(Haken::ccM27) |
    TABLE_BIT(Haken::ccM28) |
    TABLE_BIT(Haken::ccM29) |
    TABLE_BIT(Haken::ccM30);
#define HI_TABLE_BIT(m) (uint64_t(1) << (uint64_t(m) - 64))
constexpr const uint64_t HI_14BIT_CC =
    HI_TABLE_BIT(Haken::ccBrightness) |
    HI_TABLE_BIT(Haken::ccReci5) |
    HI_TABLE_BIT(Haken::ccReci6) |
    HI_TABLE_BIT(Haken::ccM31) |
    HI_TABLE_BIT(Haken::ccM32) |
    HI_TABLE_BIT(Haken::ccM33) |
    HI_TABLE_BIT(Haken::ccM34) |
    HI_TABLE_BIT(Haken::ccM35) |
    HI_TABLE_BIT(Haken::ccM36) |
    HI_TABLE_BIT(Haken::ccM37) |
    HI_TABLE_BIT(Haken::ccM38) |
    HI_TABLE_BIT(Haken::ccM39) |
    HI_TABLE_BIT(Haken::ccM40) |
    HI_TABLE_BIT(Haken::ccM41) |
    HI_TABLE_BIT(Haken::ccM42) |
    HI_TABLE_BIT(Haken::ccM43) |
    HI_TABLE_BIT(Haken::ccM44) |
    HI_TABLE_BIT(Haken::ccM45) |
    HI_TABLE_BIT(Haken::ccM46) |
    HI_TABLE_BIT(Haken::ccM47) |
    HI_TABLE_BIT(Haken::ccM48);

// Only channels 1-15
bool is_14bit_cc(uint8_t cc)
{
    if (cc < 64) return 0 != (TABLE_BIT(cc) & LO_14BIT_CC);
    return 0 != (HI_TABLE_BIT(cc) & HI_14BIT_CC);
}
#undef TABLE_BIT
#undef HI_TABLE_BIT

const char * scan_whitespace(const char *scan)
{
    while (std::isspace(*scan)) { ++scan; }
    return scan;
}

const char * scan_comment(const char *scan)
{
    if ('"' == *scan) {
        ++scan;
        while (*scan && '"' != *scan) {
            ++scan;
        }
        if ('"' == *scan) ++scan;
    }
    return scan;
}

const char * scan_whitespace_or_comment(const char *scan)
{
    while (*scan && (('"' == *scan) || (std::isspace(*scan)))) {
        scan = ('"' == *scan) ? scan_comment(scan) : scan_whitespace(scan);
    }
    return scan;
}

inline bool valid_number(uint32_t raw_number, NumberSize size)
{
    return raw_number <= static_cast<uint32_t>(size);
}

const char * bitness(NumberSize size)
{
    switch (size) {
    case NumberSize::SevenBit: return "7-bit";
    case NumberSize::FourteenBit: return "14-bit";
    default: return "";
    }
}

// Lookup depends on ascending order
uint32_t reserved[] = {
    Reserved::Macro,
    Reserved::Stream,
    Reserved::Cc,
    Reserved::ProgramChange,
    Reserved::Channel,
    Reserved::BiqSin_poke,
    Reserved::Formula_poke,
    Reserved::Graph_poke,
    Reserved::Kinetic_poke,
    Reserved::Matrix_poke,
    Reserved::Conv_poke,
    0
};

bool is_reserved(uint32_t op)
{
    #ifndef NDEBUG
    uint32_t last = 0;
    for (auto p = &reserved[0]; *p; ++p) {
        if (last) {
            assert(last < *p);
        }
        last = *p;
    }
    #endif
    if (op) {
        for (auto p = &reserved[0]; *p; ++p) {
            if (op < *p) return false;
            if (op == *p) return true;
        }
    }
    return false;
}

size_t op_len(uint32_t op) {
    if (op < Reserved::Cc) {
        return op ? 1 : 0;
    }
    return 2;
}

inline bool is_reserved(Opcode op) { return is_reserved(op.nom()); }

#define OP_BIT(c) (1 << ((c) - 'b'))
constexpr const uint32_t OP_BITS =
    1
    | OP_BIT('c')
    | OP_BIT('f')
    | OP_BIT('g')
    | OP_BIT('h')
    | OP_BIT('k')
    | OP_BIT('m')
    | OP_BIT('p')
    | OP_BIT('s')
    | OP_BIT('v')
    ;

bool is_op_char(char c)
{
    return in_range(c, 'b', 'v') ? (0 != (OP_BITS & OP_BIT(c))) : false;
}
#undef OP_BIT

Opcode get_maybe_opcode(const char * scan)
{
    Opcode op;
    auto end = scan;
    char * poke = &op.code.name[0];
    while (is_op_char(*end)) {
        *poke++ = *scan++;
        ++end;
        if ((end - scan) > 3) return Opcode{};
    }
    return op;
}

Opcode get_valid_opcode(const char * scan)
{
    auto op = get_maybe_opcode(scan);
    return is_reserved(op) ? op : Opcode{};
}

uint8_t macro_cc(uint8_t macro_number)
{
    assert(in_range(int(macro_number), 1, 90));
    uint8_t offset = macro_number - 1;
    if (macro_number <  7) return offset + Haken::ccI;
    if (macro_number < 31) return offset + Haken::ccM7;
    if (macro_number < 49) return offset + Haken::ccM31;
    if (macro_number < 73) return offset - 49 + Haken::ccM49;
    return offset - 73 + Haken::ccM73;
}

bool is_rational_start(char c) {
    return
        ('+' == c)
        || ('-' == c)
        || ('.' == c)
        || (',' == c)
        ;
}

bool is_name_start(char c) {
    return ('_' == c) || std::isalpha(c);
}

bool is_name_char(char c) {
    return ('_' == c)
        || ('-' == c)
        || ('$' == c)
        || std::isalnum(c)
        ;
}

const char * HclCompiler::scan_opcode(const char *scan)
{
    code = get_valid_opcode(scan);
    if (!code.nom()) {
        error(format_string("Expected channel, cc, macro, stream, or poke"), scan);
        return scan;
    }
    scan += op_len(code.nom());
    return scan;
}

const char * HclCompiler::scan_number(const char *scan, NumberSize size)
{
    value = 0;
    bool seven_bit = ('\'' == *scan);
    if (seven_bit) {
        ++scan;
    }
    if (!std::isdigit(*scan)) {
        error("Number expected", scan);
        return scan;
    }

    uint32_t number{0};
    while (*scan) {
        if (std::isdigit(*scan)) {
            number = (number*10) + (*scan - '0');
        } else if ('_' != *scan) {
            break;
        }
        ++scan;
    }
    ok = valid_number(number, seven_bit ? NumberSize::SevenBit : size);
    if (ok) {
        value = (seven_bit && (size == NumberSize::FourteenBit)) ? (number << 7) : number;
    } else {
        error(format_string("%lld is too large for expected %s number", number, bitness(size)), scan);
    }
    return scan;
}

const char * HclCompiler::scan_var_number(const char *scan, NumberSize size)
{
    value = 0;
    scan = scan_whitespace_or_comment(scan);
    if (is_name_start(*scan)) {
        auto start = scan++;
        while (*scan && is_name_char(*scan)) {
            ++scan;
        }
        std::string var{start, scan};
        if ((NumberSize::FourteenBit == size) && (0 == var.compare("zero"))) {
            value = Haken::zero14;
        } else {
            auto it = variables.find(var);
            if (it == variables.end()) {
                error(format_string("Undefined variable %s", var.c_str()), scan);
            } else {
                value = it->second;
                if (!valid_number(value, size)) {
                    error(format_string("Variable %s value %d out of range for %s", var.c_str(), value, bitness(size)), scan);
                    value = 0;
                }
            }
        }
        return scan;
    }
    if ((size == NumberSize::FourteenBit) && is_rational_start(*scan)) {
        return scan_rational(scan);
    }
    return scan_number(scan, size);
}

const char *scan_fraction(const char * scan, double* result)
{
    *result = NAN;
    if (('.' != *scan) && (',' != *scan)) {
        return scan;
    }
    ++scan;
    auto start = scan;
    double place = 0.1;
    double f = 0;
    while (*scan) {
        if (std::isdigit(*scan)) {
            f += place * (*scan - '0');
            place *= 0.1;
            ++scan;
        } else {
            break;
        }
    }
    if (start != scan) {
        *result = f;
    }
    return scan;
}

const char *HclCompiler::scan_rational(const char *scan)
{
    double f_value{NAN};
    switch (*scan) {
        case '-':
            ++scan;
            if ('1' == *scan) {
                value = 0;
                return scan + 1;
            } else {
                scan = scan_fraction(scan, &f_value);
            }
            break;

        case '+':
            ++scan;
            if ('1' == *scan) {
                value = Haken::max14;
                return scan + 1;
            } else {
                scan = scan_fraction(scan, &f_value);
            }
            break;

        case '.':
        case ',':
            scan = scan_fraction(scan, &f_value);
            break;

        default:
            ok = false;
            break;
    }
    if (std::isnan(f_value)) {
        ok = false;
    }
    if (ok) {
        f_value = std::round((f_value / inv_zero14) + Haken::zero14);
        value = f_value;
    } else {
        error("Expected -1 .. 1 value", scan);
    }
    return scan;
}

const char * HclCompiler::scan_variable_def(const char *scan)
{
    assert('{' == *scan);
    ++scan;
    const char * name_start{nullptr};
    const char * name_end{nullptr};
    while (*scan) {
        switch (*scan) {

        case '}': return ++scan;

        case ';': ++scan; break;

        case ' ': case '\t': case '\r': case '\n':
            scan = scan_whitespace(scan);
            break;

        case '=':
            if (!name_start) {
                error("Variable name expected", scan);
                return scan;
            }
            name_end = scan;
            scan = scan_number(++scan, NumberSize::FourteenBit);
            if (ok) {
                auto key = std::string(name_start, name_end);
                auto it = variables.lower_bound(key);
                if (it == variables.end()) {
                    variables.insert(std::make_pair(key, value));
                } else {
                    it->second = value;
                }
                name_start = name_end = nullptr;
            }
            break;

        default:
            if (!name_start) {
                if (is_name_start(*scan)) {
                    name_start = scan;
                } else {
                    error("Variable names must begin with character in the set [a-zA-Z$_]", scan);;
                    return scan;
                }
            } else if (!is_name_char(*scan)) {
                error("Variable names can contain only characters in the set [a-zA-Z0-9$_-]", scan);;
                return scan;
            }
            ++scan;
            break;
        }
    }
    return scan;
}

const char * HclCompiler::scan_channel(const char *scan)
{
    scan = scan_whitespace_or_comment(scan);
    scan = scan_number(scan, NumberSize::SevenBit);
    if (ok) {
        if (in_range(int(value), 1, 16)) {
            channel = value - 1;
        } else {
            error(format_string("Channel %d is out of range. Expected 1 .. 16)", value), scan);
        }
    }
    return scan;
}

const char * HclCompiler::scan_cc(const char *scan)
{
    scan = scan_whitespace_or_comment(scan);
    scan = scan_number(scan, NumberSize::SevenBit);
    if (ok) {
        uint8_t cc = value;
        scan = scan_whitespace_or_comment(scan);
        scan = scan_var_number(scan, NumberSize::FourteenBit);
        if (ok) {
            if (channel == Haken::ch16) {
                if (value > 127) {
                    ok = false;
                    error_message = format_string("Not supported: 14-bit CC %d on channel 16", cc);
                    return scan;
                }
            }
            if (dest) {
                PackedMidiMessage m;
                if (channel == Haken::ch16) {
                    m = Tag(MakeCC(channel, cc, value), U8(ChemId::MidiPad));
                    dest->push_back(m);
                } else {
                    if (is_14bit_cc(cc)) {
                        if (value > 127) {
                            uint8_t lo = value & 0x7f;
                            uint8_t hi = value >> 7;
                            m = Tag(MakeCC(channel, Haken::ccFracIM48, lo), U8(ChemId::MidiPad));
                            dest->push_back(m);
                            m = Tag(MakeCC(channel, cc, hi), U8(ChemId::MidiPad));
                            dest->push_back(m);
                        } else {
                            m = Tag(MakeCC(channel, Haken::ccFracIM48, 0), U8(ChemId::MidiPad));
                            dest->push_back(m);
                            m = Tag(MakeCC(channel, cc, value), U8(ChemId::MidiPad));
                            dest->push_back(m);
                        }
                    } else {
                        if (value <= 127) {
                            m = Tag(MakeCC(channel, cc, value), U8(ChemId::MidiPad));
                            dest->push_back(m);
                        } else {
                            ok = false;
                            error_message = format_string("%d out of range for 7-bit cc %d", value, cc);
                        }
                    }
                }
            }
        }
    }
    return scan;
}

const char *HclCompiler::scan_pc(const char *scan)
{
    scan = scan_whitespace_or_comment(scan);
    scan = scan_number(scan, NumberSize::SevenBit);
    if (ok) {
        uint8_t pc = value;
        auto m = Tag(MakeProgramChange(channel, pc), U8(ChemId::MidiPad));
        dest->push_back(m);
    }
    return scan;
}

const char *HclCompiler::scan_macro(const char *scan)
{
    scan = scan_whitespace_or_comment(scan);
    scan = scan_number(scan, NumberSize::SevenBit);
    if (ok) {
        if (!in_range(int(value), 1, 90)) {
            ok = false;
            error(format_string("Macro # %d is out of range. Expected 1 .. 90)", value), scan);
            return scan;
        }
        uint8_t macro_number = value;
        scan = scan_whitespace_or_comment(scan);
        scan = scan_var_number(scan, NumberSize::FourteenBit);
        if (ok) {
            uint8_t lo = value & 0x7f;
            uint8_t hi = value >> 7;

            uint8_t ccFrac = macro_number < 49 ? Haken::ccFracIM48 : Haken::ccFracM49M90;
            bool omit_lo = (0 == lo) && (macro_number < 7);
            if (!omit_lo) {
                auto m = Tag(MakeCC(channel, ccFrac, lo), U8(ChemId::MidiPad));
                dest->push_back(m);
            }

            auto cc = macro_cc(macro_number);
            auto m = Tag(MakeCC(channel, cc, hi), U8(ChemId::MidiPad));
            dest->push_back(m);
        }
    }
    return scan;
}

const char * HclCompiler::scan_stream(const char *scan)
{
    scan = scan_number(scan, NumberSize::SevenBit);
    if (ok) {
        if (in_range(int(value), 0, 27)) {
            auto m = Tag(MakeCC(Haken::ch16, Haken::ccStream, value), U8(ChemId::MidiPad));
            dest->push_back(m);
            scan = scan_list(scan);
        } else {
            error(format_string("Invalid stream id %d", value), scan);
        }
    }
    return scan;
}

const char *HclCompiler::scan_graph(const char *scan)
{
    if (' ' == *scan) {
        return scan_poke(scan, Haken::s_Graph_Poke);
    } else {
        scan = scan_number(scan, NumberSize::SevenBit);
        if (ok) {
            if (in_range(int(value), 0, 2)) {
                return scan_poke(scan, Haken::s_Graph_Poke + value);
            } else {
                error(format_string("Invalid graph number %d. Must be 0, 1, or 2", value), scan);
            }
        }
    }
    return scan;
}

const char *HclCompiler::scan_poke(const char *scan, uint8_t poke_stream)
{
    if (dest) {
        auto m = Tag(MakeCC(Haken::ch16, Haken::ccStream, poke_stream), U8(ChemId::MidiPad));
        dest->push_back(m);
    }
    return scan_list(scan);
}

const char * HclCompiler::scan_list(const char *scan)
{
    scan = scan_whitespace_or_comment(scan);
    if ('[' != *scan) {
        error("Expected list: [...]", scan);
        return scan;
    }
    ++scan;
    scan = scan_whitespace_or_comment(scan);

    uint16_t first;
    uint16_t second;
    while (*scan && (']' != *scan) && ok) {
        first = second = 0;
        scan = scan_var_number(scan, NumberSize::SevenBit);
        if (ok) {
            first = value;
            scan = scan_whitespace_or_comment(scan);
            if (']' == *scan) {
                    // unpaired
                    if (code.nom() != Reserved::Stream) {
                        error(format_string("Opcode %s requires an even number of values", code.str()), scan);
                    }
            } else {
                scan = scan_var_number(scan, NumberSize::SevenBit);
                second = value;
            }
            if (ok) {
                if (dest) {
                    auto m = Tag(MakePolyKeyPressure(Haken::ch16, first, second), U8(ChemId::MidiPad));
                    dest->push_back(m);
                }
            } else {
                return scan;
            }
        }
    }
    if (ok && *scan != ']') {
        error("Expected list end: ']'", scan);
        return scan;
    }
    ++scan;
    return scan;
}

const char * HclCompiler::parse_opcode(const char *scan)
{
    switch (code.nom()) {
    case Reserved::Macro: return scan_macro(scan);
    case Reserved::Stream: return scan_stream(scan);
    case Reserved::ProgramChange: return scan_pc(scan);
    case Reserved::Cc: return scan_cc(scan);
    case Reserved::Channel: return scan_channel(scan);
    case Reserved::BiqSin_poke: return scan_poke(scan, Haken::s_BiqSin_Poke);
    case Reserved::Formula_poke: return scan_poke(scan, Haken::s_Form_Poke);
    case Reserved::Graph_poke: return scan_graph(scan);
    case Reserved::Kinetic_poke: return scan_poke(scan, Haken::s_Kinet_Poke);
    case Reserved::Matrix_poke: return scan_poke(scan, Haken::s_Mat_Poke);
    case Reserved::Conv_poke: return scan_poke(scan, Haken::s_Conv_Poke);
    default:
        error(format_string("Unknown opcode %s", code.str()), scan);
        break;
    }
    return scan;
}

void HclCompiler::error(const std::string& error, const char *pos)
{
    ok = false;
    error_message = error;
    error_pos = pos - program_start;
}

bool HclCompiler::compile(const std::string &program, std::vector<PackedMidiMessage> *midi)
{
    dest = midi;
    if (dest) dest->clear();
    program_start = program.c_str();
    const char * scan = program_start;
    if (program.empty()) {
        error("Midi definition is empty.", scan);
        return false;
    }
    while (ok && *scan) {
        scan = scan_whitespace_or_comment(scan);
        if ('{' == *scan) {
            scan = scan_variable_def(scan);
        } else if (*scan) {
            scan = scan_opcode(scan);
            if (ok) {
                scan = parse_opcode(scan);
            }
        }
    }
    // if (dest && dest->empty()) {
    //     error("Compilation produced no MIDI messages", scan);
    // }
    return ok;
}

};