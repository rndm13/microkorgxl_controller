#pragma once

#include <stdint.h>

#include "settings.hpp"

#define PROGRAM_SERIALIZED_SIZE      346
#define PROGRAM_NAME_LENGTH          8

#define TIMBRE_NUM                   2

struct Filter {
    FilterTypeBalance balance;
    int cutoff;
    int resonance;
};

struct Timbre {
    Filter filter_1;
};

struct Program {
    char name[PROGRAM_NAME_LENGTH + 1];
    Timbre timbre_arr[TIMBRE_NUM];
};

struct __attribute__((packed)) SerializedOsc1Data {
    uint8_t wave_type : 4;
    uint8_t osc_mod : 2;
    uint8_t _dummy0 : 2;

    uint8_t wave_ctrl[2];

    uint8_t pcm_dwgs_wave_select;

    uint8_t _dummy1;
};

struct __attribute__((packed)) SerializedOsc2Data {
    uint8_t wave_type : 2;
    uint8_t _dummy0 : 2;
    uint8_t osc_mod : 2;
    uint8_t _dummy1 : 2;

    uint8_t semitone;
    uint8_t tune;
};

struct __attribute__((packed)) SerializedMixerData {
    uint8_t osc_level_arr[2];
    uint8_t noise_level;

    uint8_t _dummy0;
};

struct __attribute__((packed)) SerializedFilter {
    uint8_t cutoff;
    uint8_t resonance;
    uint8_t eg1_int;
    uint8_t key_track;
    uint8_t vel_sens;
};

struct __attribute__((packed)) SerializedFilterData {
    uint8_t filter_routing : 2;
    uint8_t _dummy0 : 2;
    uint8_t filter2_type : 2;
    uint8_t _dummy1 : 2;

    uint8_t filter1_bal;
    SerializedFilter filters[2];
};

struct __attribute__((packed)) SerializedAmpData {
    uint8_t level;

    uint8_t _dummy0 : 4;
    uint8_t drive_ws_pos : 2;
    uint8_t _dummy1 : 2;

    uint8_t wave_type : 4;
    uint8_t _dummy2 : 4;

    uint8_t depth;
    uint8_t panpot;
    uint8_t key_track;
    uint8_t punch_level;
    uint8_t _dummy3;
};

struct __attribute__((packed)) SerializedEG {
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
    uint8_t level_velocity_sens;
    uint8_t _dummy0;
};

struct __attribute__((packed)) SerializedLFO {
    uint8_t wave_type : 4;
    uint8_t _dummy0 : 4;

    uint8_t freq;

    uint8_t _dummy1 : 5;
    uint8_t key_sync : 2;
    uint8_t bpm_sync : 1;

    uint8_t sync_note : 4;
    uint8_t _dummy2 : 4;
};

struct __attribute__((packed)) SerializedPatch {
    uint8_t src;
    uint8_t dst;
    uint8_t intensity;
};

struct __attribute__((packed)) SerializedProgramData {
    uint8_t knob_assign[3];
    uint8_t _dummy0;

    uint8_t unison_voice : 4;
    uint8_t _dummy1 : 4;

    uint8_t unison_detune;
    uint8_t unison_spread;

    uint8_t _dummy2 : 6;
    uint8_t voice_assign : 2;

    uint8_t _dummy3;

    uint8_t analog_tuning;
    uint8_t transpose;
    uint8_t detune;
    uint8_t vibrato_int;
    uint8_t bend_range;
    uint8_t portamento_time;

    uint8_t _dummy4;

    SerializedOsc1Data osc1;
    SerializedOsc2Data osc2;
    SerializedMixerData mixer;
    SerializedFilterData filter;
    SerializedAmpData amp;
    SerializedEG eg_arr[3];
    SerializedLFO lfo_arr[2];
    SerializedPatch patch_arr[6];
};

struct __attribute__((packed)) SerializedTimbre {
    SerializedProgramData data;

    uint8_t eq_lfreq;
    uint8_t eq_lgain;

    uint8_t eq_hfreq;
    uint8_t eq_hgain;
};

struct __attribute__((packed)) SerializedVocoder {
    char data[78];
};

struct __attribute__((packed)) SerializedEffects {
    char data[48];
};

struct __attribute__((packed)) SerializedArppegio {
    uint8_t type : 3;
    uint8_t _dummy0 : 1;
    uint8_t resolution : 4;

    uint8_t last_step : 5;
    uint8_t octave_range : 2;
    uint8_t latch : 1;

    uint8_t gate_time;
    uint8_t swing;

    uint8_t step_mask;
    uint8_t _dummy1[3];
};

struct __attribute__((packed)) SerializedProgram {
    char name[PROGRAM_NAME_LENGTH];

    uint8_t _dummy0 : 4;
    uint8_t arp_timb_select : 2;
    uint8_t voice_mode : 2;

    uint8_t _dummy1[3];

    uint8_t scale_type : 4;
    uint8_t scale_key : 4;

    uint8_t timbre2_midi_channel;

    uint8_t _dummy2;

    uint8_t _dummy3 : 4;
    uint8_t octave_sw : 4;

    SerializedTimbre timbre_arr[TIMBRE_NUM];
    SerializedVocoder vocoder;
    SerializedEffects effects;

    uint16_t tempo;

    uint8_t _dummy4 : 6;
    uint8_t key_sync_on : 1;
    uint8_t arp_on : 1;

    uint8_t _dummy5;

    SerializedArppegio arp;
};

void program_deserialize(Program* out_prog, const uint8_t* data);
void program_serialize(const Program* prog, uint8_t* data);
