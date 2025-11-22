#pragma once

#include <stdint.h>

#define PROGRAM_SERIALIZED_SIZE      342
#define PROGRAM_NAME_LENGTH          8

#define TIMBRE_NUM                   2

struct Filter {
    int cutoff;
    int resonance;
    int type_bal;
    int eg1_int;
    int key_trk;
    int vel_sens;
};

struct Oscillator {
    // Osc 2 has different type values
    int wave;
    // Osc 2 has different ones
    int osc_mod;
    // Osc2 doesn't have this
    int control_arr[2];
};

struct Unison {
    int mode;
    // TODO: Add detune, spread voice assign
};

struct Mixer {
    int osc_lvl[2];
    int noise_lvl;
};

struct Amp {
    int level;
    int depth;
    int panpot;
};

struct EnvelopeGenerator {
    int attack;
    int decay;
    int sustain;
    int release;
    int vel_sens;
};

struct LFO {
    int wave;
    int freq;
};

struct Patch {
    int src;
    int dst;
    int intensty;
};

struct Equalizer {
    int lo_gain;
    int hi_gain;
};

struct Effect {
    int dry_wet;
    int ctrl_1;
    int ctrl_2;
};

struct Timbre {
    // Filter 2 doesn't have THRU, and LPF24
    int filter_routing;
    Filter filter_arr[2];
    Oscillator osc_1;
    Unison unison;
    Mixer mixer;
    Amp amp;
    EnvelopeGenerator eg_arr[3];
    LFO lfo_arr[2];
    Patch patch_arr[6];
    Equalizer eq;
};

struct Program {
    char name[PROGRAM_NAME_LENGTH];
    Timbre timbre_arr[TIMBRE_NUM];
};

void program_deserialize(Program& out_prog, uint8_t* data);
void program_serialize(const Program& prog, uint8_t* data);
