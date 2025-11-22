#pragma once

#include <stdint.h>

#define PROGRAM_SERIALIZED_SIZE      342
#define PROGRAM_NAME_LENGTH          8

struct Filter {
    int cutoff;
    int resonance;
    int type_bal;
    int eg1_int;
    int key_trk;
    int vel_sens;
};

struct Oscillator {
    int wave;
    int osc_mod;
    int osc1c1;
    int osc1c2;
};

struct Unison {
    int mode;
};

struct Mixer {
    int osc1_lvl;
    int noise_lvl;
};

struct Amp {
    int level;
    int panpot;
};

struct DriveWS {
    int ws_depth;
};

struct EnvelopeGenerator {
    int attack;
    int decay;
    int sustain;
    int release;
};

struct LFO {
    int wave;
    int freq;
};

struct Patch {
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
    DriveWS drive_ws;
    EnvelopeGenerator eg_1;
    LFO lfo_1;
    Patch patch_1;
    Equalizer eq;
    Effect mst_fx_1;
};

struct Program {
    char name[PROGRAM_NAME_LENGTH];
    Timbre timbre_1;
    Timbre timbre_2;
};

void program_deserialize(Program& out_prog, uint8_t* data);
void program_serialize(const Program& prog, uint8_t* data);
