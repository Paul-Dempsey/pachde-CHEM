#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../em/midi-message.h"

namespace pachde {

constexpr const uint8_t PKP16 = Haken::polyKeyPres + Haken::ch16;

struct ConvolutionParams
{
    uint8_t data[30];
    bool in_conv_stream;

    ConvolutionParams();

    uint8_t get_pre_index() { return data[Haken::id_c_idx1]; }
    uint8_t get_pre_mix() { return data[Haken::id_c_mix1]; }
    uint8_t get_post_index() { return data[Haken::id_c_idx2]; }
    uint8_t get_post_mix() { return data[Haken::id_c_mix2]; }

    void set_pre_index  (uint8_t value) { data[Haken::id_c_idx1] = value; }
    void set_pre_mix    (uint8_t value) { data[Haken::id_c_mix1] = value; }
    void set_post_index (uint8_t value) { data[Haken::id_c_idx2] = value; }
    void set_post_mix   (uint8_t value) { data[Haken::id_c_mix2] = value; }
    
    uint8_t get_ir_type (int index) { return data[Haken::id_c_dat0 + index]; }
    uint8_t get_ir1_type () { return data[Haken::id_c_dat0]; }
	uint8_t get_ir2_type () { return data[Haken::id_c_dat1]; }
	uint8_t get_ir3_type () { return data[Haken::id_c_dat2]; }
	uint8_t get_ir4_type () { return data[Haken::id_c_dat3]; }

    void set_ir_type (int index, uint8_t value) { data[Haken::id_c_dat0 + index] = value; }
    void set_ir1_type (uint8_t value) { data[Haken::id_c_dat0] = value; }
	void set_ir2_type (uint8_t value) { data[Haken::id_c_dat1] = value; }
	void set_ir3_type (uint8_t value) { data[Haken::id_c_dat2] = value; }
	void set_ir4_type (uint8_t value) { data[Haken::id_c_dat3] = value; }

    uint8_t get_ir_length (int index) { return data[Haken::id_c_lth0 + index]; }
    uint8_t get_ir1_length () { return data[Haken::id_c_lth0]; }
	uint8_t get_ir2_length () { return data[Haken::id_c_lth1]; }
	uint8_t get_ir3_length () { return data[Haken::id_c_lth2]; }
	uint8_t get_ir4_length () { return data[Haken::id_c_lth3]; }

    void set_ir_length (int index, uint8_t value) { data[Haken::id_c_lth0 + index] = value; }
    void set_ir1_length (uint8_t value) { data[Haken::id_c_lth0] = value; }
	void set_ir2_length (uint8_t value) { data[Haken::id_c_lth1] = value; }
	void set_ir3_length (uint8_t value) { data[Haken::id_c_lth2] = value; }
	void set_ir4_length (uint8_t value) { data[Haken::id_c_lth3] = value; }

    uint8_t get_ir_shift (int index) { return data[Haken::id_c_shf0 + index]; }
	uint8_t get_ir1_shift() { return data[Haken::id_c_shf0]; }
	uint8_t get_ir2_shift() { return data[Haken::id_c_shf1]; }
	uint8_t get_ir3_shift() { return data[Haken::id_c_shf2]; }
	uint8_t get_ir4_shift() { return data[Haken::id_c_shf3]; }

    void set_ir_shift (int index, uint8_t value) { data[Haken::id_c_shf0 + index] = value; }
	void set_ir1_shift(uint8_t value) { data[Haken::id_c_shf0] = value; }
	void set_ir2_shift(uint8_t value) { data[Haken::id_c_shf1] = value; }
	void set_ir3_shift(uint8_t value) { data[Haken::id_c_shf2] = value; }
	void set_ir4_shift(uint8_t value) { data[Haken::id_c_shf3] = value; }

    uint8_t get_ir_width (int index) { return data[Haken::id_c_wid0 + index]; }
	uint8_t get_ir1_width () { return data[Haken::id_c_wid0]; }
	uint8_t get_ir2_width () { return data[Haken::id_c_wid1]; }
	uint8_t get_ir3_width () { return data[Haken::id_c_wid2]; }
	uint8_t get_ir4_width () { return data[Haken::id_c_wid3]; }

    void set_ir_width (int index, uint8_t value) { data[Haken::id_c_wid0 + index] = value; }
	void set_ir1_width (uint8_t value) { data[Haken::id_c_wid0] = value; }
	void set_ir2_width (uint8_t value) { data[Haken::id_c_wid1] = value; }
	void set_ir3_width (uint8_t value) { data[Haken::id_c_wid2] = value; }
	void set_ir4_width (uint8_t value) { data[Haken::id_c_wid3] = value; }

    uint8_t get_ir_left (int index) { return data[Haken::id_c_atL0 + index]; }
    uint8_t get_ir1_left () { return data[Haken::id_c_atL0]; }
	uint8_t get_ir2_left () { return data[Haken::id_c_atL1]; }
	uint8_t get_ir3_left () { return data[Haken::id_c_atL2]; }
	uint8_t get_ir4_left () { return data[Haken::id_c_atL3]; }

    void set_ir_left (int index, uint8_t value) { data[Haken::id_c_atL0 + index] = value; }
    void set_ir1_left (uint8_t value) { data[Haken::id_c_atL0] = value; }
	void set_ir2_left (uint8_t value) { data[Haken::id_c_atL1] = value; }
	void set_ir3_left (uint8_t value) { data[Haken::id_c_atL2] = value; }
	void set_ir4_left (uint8_t value) { data[Haken::id_c_atL3] = value; }

    uint8_t get_ir_right (int index) { return data[Haken::id_c_atR0 + index]; }
    uint8_t get_ir1_right () { return data[Haken::id_c_atR0]; }
	uint8_t get_ir2_right () { return data[Haken::id_c_atR1]; }
	uint8_t get_ir3_right () { return data[Haken::id_c_atR2]; }
	uint8_t get_ir4_right () { return data[Haken::id_c_atR3]; }

    void set_ir_right (int index, uint8_t value) { data[Haken::id_c_atR0 + index] = value; }
    void set_ir1_right (uint8_t value) { data[Haken::id_c_atR0] = value; }
	void set_ir2_right (uint8_t value) { data[Haken::id_c_atR1] = value; }
	void set_ir3_right (uint8_t value) { data[Haken::id_c_atR2] = value; }
	void set_ir4_right (uint8_t value) { data[Haken::id_c_atR3] = value; }

    uint8_t get_phase_cancellation () { return data[Haken::id_c_phc]; }
    void set_phase_cancellation (uint8_t value) { data[Haken::id_c_phc] = value; }

    void do_message(PackedMidiMessage message);
};

}