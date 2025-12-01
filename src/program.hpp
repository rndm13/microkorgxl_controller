#pragma once

#include <stdint.h>

#define PROGRAM_SERIALIZED_SIZE      346
#define PROGRAM_MIDI_SERIALIZED_SIZE 396
#define PROGRAM_DUMP_SEND_SIZE       399

#define PROGRAM_NAME_LENGTH          8

#define TIMBRE_NUM                   2

#define FILE_PROGRAM_STR_HEADER   "6140PrgD"
#define FILE_PROGRAM_HEADER       "\x36\x31\x34\x30\x50\x72\x67\x44\x20\x00\xF0\x01\x00\x00\x01\x00\x02\x00\x00\x00\x01\x00\x00\x00\xF0\x01\x00\x00\xFF\xFF\xFF\xFF"
#define FILE_PROGRAM_HEADER_SIZE  0x20
#define FILE_PROGRAM_FOOTER_SIZE  0x16
#define FILE_WEIRD_DATA_SIZE      0x10
#define FILE_ZERO_PADDING_SIZE    0x49
#define FILE_255_PADDING_SIZE     0x17

struct Filter {
    int cutoff;
    int resonance;
    // Filter 2 doesn't have THRU, and LPF24
    int type_bal;
    int eg1_int;
    int key_trk;
    int vel_sens;
};

struct Oscillator {
    // Osc2 has different ones
    int wave;
    // Osc2 has different ones
    int osc_mod;
    // Osc2 doesn't have this
    int control_arr[2];
    // Osc2 doesn't have this
    int pcm_dwgs_wave;
    // Osc1 doesn't have this
    int semitone;
    // Osc1 doesn't have this
    int tune;
};

struct Unison {
    int mode;
    int detune;
    int spread;
    int voice_assign;
};

struct Mixer {
    int osc_lvl[2];
    int noise_lvl;
};

struct Amp {
    int lvl;
    int drive_ws_pos;
    int wave_type;
    int depth;
    int panpot;
    int key_trk;
    int punch_lvl;
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
    int key_sync;
    int bpm_sync;
    int note_sync;
};

struct Patch {
    int src;
    int dst;
    int intensity;
};

struct Equalizer {
    int lo_gain;
    int lo_freq;
    int hi_gain;
    int hi_freq;
};

struct Effect {
    int dry_wet;
    int ctrl_1;
    int ctrl_2;
};

struct PitchData {
    int analog_tuning;
    int transpose;
    int detune;
    int vibrato_int;
    int bend_range;
};

struct Timbre {
    int filter_routing;
    Filter filter_arr[2];
    Oscillator osc_arr[2];
    Unison unison;
    PitchData pitch;
    Mixer mixer;
    Amp amp;
    EnvelopeGenerator eg_arr[3];
    LFO lfo_arr[2];
    Patch patch_arr[6];
    Equalizer eq;
};

struct Program {
    char name[PROGRAM_NAME_LENGTH + 1];
    Timbre timbre_arr[TIMBRE_NUM];

    int voice_mode;
    int arp_timb_select;
    int scale_key;
    int scale_type;
    int timbre2_midi_channel;
    int octave_sw;

    uint16_t tempo;
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
    uint8_t osc_lvl_arr[2];
    uint8_t noise_lvl;

    uint8_t _dummy0;
};

struct __attribute__((packed)) SerializedFilter {
    uint8_t cutoff;
    uint8_t resonance;
    uint8_t eg1_int;
    uint8_t key_trk;
    uint8_t vel_sens;
};

struct __attribute__((packed)) SerializedFilterData {
    uint8_t filter_routing : 2;
    uint8_t _dummy0 : 2;
    uint8_t filter2_type : 2;
    uint8_t _dummy1 : 2;

    uint8_t filter1_bal;
    SerializedFilter filter_arr[2];
};

struct __attribute__((packed)) SerializedAmpData {
    uint8_t lvl;

    uint8_t _dummy0 : 4;
    uint8_t drive_ws_pos : 2;
    uint8_t _dummy1 : 2;

    uint8_t wave_type : 4;
    uint8_t _dummy2 : 4;

    uint8_t depth;
    uint8_t panpot;
    uint8_t key_trk;
    uint8_t punch_lvl;
    uint8_t _dummy3;
};

struct __attribute__((packed)) SerializedEG {
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
    uint8_t vel_sens;
    uint8_t _dummy0;
};

struct __attribute__((packed)) SerializedLFO {
    uint8_t wave_type : 4;
    uint8_t _dummy0 : 4;

    uint8_t freq;

    uint8_t _dummy1 : 5;
    uint8_t key_sync : 2;
    uint8_t bpm_sync : 1;

    uint8_t note_sync : 4;
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

struct FileSerializedProgram {
    uint8_t header[0x20];

    SerializedProgram prog;

    uint8_t footer[0x16];
    uint8_t weird[0x10];
    uint8_t pad_zero[0x49];
    uint8_t pad_255[0x17];
};

void program_deserialize(Program* out_prog, const uint8_t* data);
void program_serialize(const Program* prog, uint8_t* data);
int program_save(const Program* prog, const char* filepath);
int program_open(Program* prog, const char* filepath);
