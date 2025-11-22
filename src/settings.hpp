#pragma once

#include <stddef.h>

#define ARRAY_SIZE(X) (sizeof(X) / (sizeof(*(X))))

#define TIMBRE_1_ID      0x10
#define TIMBRE_2_ID      0x20

#define ENUM_VARIANT(NAME, VALUE) NAME = VALUE,
#define PLUS_ONE_VARIANT(...) + 1

#define MAX_ENUM_ELEMS 40


#define FILTER1_TYPE_BAL_ENUM(VARIANT)                               \
    VARIANT(FTB1_24LPF, 0)                                           \
    VARIANT(FTB1_12LPF, 31)                                          \
    VARIANT(FTB1_HPF, 63)                                            \
    VARIANT(FTB1_BPF, 95)                                            \
    VARIANT(FTB1_THRU, 127)                                          \

enum Filter1TypeBalance {
    FILTER1_TYPE_BAL_ENUM(ENUM_VARIANT)
};


#define FILTER2_TYPE_BAL_ENUM(VARIANT)                               \
    VARIANT(FTB2_12LPF, 0)                                           \
    VARIANT(FTB2_HPF, 1)                                             \
    VARIANT(FTB2_BPF, 2)                                             \

enum Filter2TypeBalance {
    FILTER2_TYPE_BAL_ENUM(ENUM_VARIANT)
};


#define OSC1_OSC_MOD_ENUM(VARIANT)                                   \
    VARIANT(OSC1_OM_WAVE, 0)                                         \
    VARIANT(OSC1_OM_CROSS, 1)                                        \
    VARIANT(OSC1_OM_UNISON, 2)                                       \
    VARIANT(OSC1_OM_VPM, 3)                                          \

enum OSC1OscMod {
    OSC1_OSC_MOD_ENUM(ENUM_VARIANT)
};


#define OSC2_OSC_MOD_ENUM(VARIANT)                                   \
    VARIANT(OSC2_OM_OFF, 0)                                          \
    VARIANT(OSC2_OM_RING, 1)                                         \
    VARIANT(OSC2_OM_SYNC, 2)                                         \
    VARIANT(OSC2_OM_RINGSYNC, 3)                                     \

enum OSC2OscMod {
    OSC2_OSC_MOD_ENUM(ENUM_VARIANT)
};


#define OSC1_WAVE_TYPE_ENUM(VARIANT)                                 \
    VARIANT(OSC1_WT_SAW,  0)                                         \
    VARIANT(OSC1_WT_PULSE, 1)                                        \
    VARIANT(OSC1_WT_TRIANGLE, 2)                                     \
    VARIANT(OSC1_WT_SIN, 3)                                          \
    VARIANT(OSC1_WT_FORMANT, 4)                                      \
    VARIANT(OSC1_WT_NOISE, 5)                                        \
    VARIANT(OSC1_WT_PCM_DWGS, 6)                                     \
    VARIANT(OSC1_WT_AUDIO_IN, 7)                                     \

enum OSC1WaveType {
    OSC1_WAVE_TYPE_ENUM(ENUM_VARIANT)
};


#define OSC2_WAVE_TYPE_ENUM(VARIANT)                                 \
    VARIANT(OSC2_WT_SAW,  0)                                         \
    VARIANT(OSC2_WT_PULSE, 1)                                        \
    VARIANT(OSC2_WT_TRIANGLE, 2)                                     \
    VARIANT(OSC2_WT_SIN, 3)                                          \

enum OSC2WaveType {
    OSC2_WAVE_TYPE_ENUM(ENUM_VARIANT)
};


#define UNISON_VOICE_ENUM(VARIANT)                                   \
    VARIANT(UV_OFF,  0)                                              \
    VARIANT(UV_2VOICE, 1)                                            \
    VARIANT(UV_4VOICE, 2)                                            \

enum UnisonVoice {
    UNISON_VOICE_ENUM(ENUM_VARIANT)
};


#define VOICE_ASSIGN_ENUM(VARIANT)                                   \
    VARIANT(VA_MONO1,  0)                                            \
    VARIANT(VA_MONO2,  1)                                            \
    VARIANT(VA_POLY,   2)                                            \

enum VoiceAssign {
    VOICE_ASSIGN_ENUM(ENUM_VARIANT)
};


#define LFO1_WAVE_ENUM(VARIANT)                                       \
    VARIANT(LFO1_W_SAW, 0)                                            \
    VARIANT(LFO1_W_SQUARE, 1)                                         \
    VARIANT(LFO1_W_TRIANGLE, 2)                                       \
    VARIANT(LFO1_W_S_AND_H, 3)                                        \
    VARIANT(LFO1_W_RANDOM,  4)                                        \

enum LFO1Wave {
    LFO1_WAVE_ENUM(ENUM_VARIANT)
};


#define LFO2_WAVE_ENUM(VARIANT)                                       \
    VARIANT(LFO2_W_SAW, 0)                                            \
    VARIANT(LFO2_W_SQUAREP, 1)                                        \
    VARIANT(LFO2_W_SINE,      2)                                      \
    VARIANT(LFO2_W_S_AND_H, 3)                                        \
    VARIANT(LFO2_W_RANDOM,  4)                                        \

enum LFO2Wave {
    LFO2_WAVE_ENUM(ENUM_VARIANT)
};


#define LFO_KEY_SYNC_ENUM(VARIANT)                                   \
    VARIANT(LFO_KS_OFF, 0)                                           \
    VARIANT(LFO_KS_TIMBRE, 1)                                        \
    VARIANT(LFO_KS_VOICE,  2)                                        \

enum LFOKeySync {
    LFO_KEY_SYNC_ENUM(ENUM_VARIANT)
};


#define VOICE_MODE_ENUM(VARIANT)                                     \
    VARIANT(VM_SINGLE, 0)                                            \
    VARIANT(VM_LAYER,  1)                                            \
    VARIANT(VM_SPLIT,  2)                                            \
    VARIANT(VM_MULTI,  3)                                            \

enum VoiceMode {
    VOICE_MODE_ENUM(ENUM_VARIANT)
};


#define ARP_TIMBRE_SELECT_ENUM(VARIANT)                              \
    VARIANT(ARP_TS_1,  0)                                            \
    VARIANT(ARP_TS_2,  1)                                            \
    VARIANT(ARP_TS_BOTH, 2)                                          \

enum ARPTimbreSelect {
    ARP_TIMBRE_SELECT_ENUM(ENUM_VARIANT)
};


#define SCALE_KEY_ENUM(VARIANT)                                      \
    VARIANT(SK_C,       0)                                           \
    VARIANT(SK_C_SHARP, 1)                                           \
    VARIANT(SK_D,       2)                                           \
    VARIANT(SK_D_SHARP, 3)                                           \
    VARIANT(SK_E,       4)                                           \
    VARIANT(SK_F,       5)                                           \
    VARIANT(SK_F_SHARP, 6)                                           \
    VARIANT(SK_G,       7)                                           \
    VARIANT(SK_G_SHARP, 8)                                           \
    VARIANT(SK_A,       9)                                           \
    VARIANT(SK_A_SHARP, 10)                                          \
    VARIANT(SK_B,       11)                                          \

enum ScaleKey {
    SCALE_KEY_ENUM(ENUM_VARIANT)
};


#define SCALE_TYPE_ENUM(VARIANT)                                     \
    VARIANT(ST_EQUAL_TEMP, 0)                                        \
    VARIANT(ST_PURE_MAJOR, 1)                                        \
    VARIANT(ST_PURE_MINOR, 2)                                        \
    VARIANT(ST_ARABIC,     3)                                        \
    VARIANT(ST_PYTHAGOREA, 4)                                        \
    VARIANT(ST_WERCKMEIST, 5)                                        \
    VARIANT(ST_KIRNBERGER, 6)                                        \
    VARIANT(ST_SLENDORO,   7)                                        \
    VARIANT(ST_PELOG,      8)                                        \
    VARIANT(ST_USER_SCALE, 9)                                        \

enum ScaleType {
    SCALE_TYPE_ENUM(ENUM_VARIANT)
};


#define PATCH_SRC_ENUM(VARIANT)                                      \
    VARIANT(PSRC_EG1,        0)                                      \
    VARIANT(PSRC_EG2,        1)                                      \
    VARIANT(PSRC_EG3,        2)                                      \
    VARIANT(PSRC_LFO1,       3)                                      \
    VARIANT(PSRC_LFO2,       4)                                      \
    VARIANT(PSRC_VELOCITY,   5)                                      \
    VARIANT(PSRC_PITCH_BEND, 6)                                      \
    VARIANT(PSRC_MOD_WHEEL,  7)                                      \
    VARIANT(PSRC_KEY_TRACK,  8)                                      \
    VARIANT(PSRC_MIDI1,      9)                                      \
    VARIANT(PSRC_MIDI2,      10)                                     \
    VARIANT(PSRC_MIDI3,      11)                                     \

enum PatchSource {
    PATCH_SRC_ENUM(ENUM_VARIANT)
};


#define PATCH_DST_ENUM(VARIANT)                                      \
    VARIANT(PDST_PITCH,                 0)                           \
    VARIANT(PDST_OSC2_TUNE,             1)                           \
    VARIANT(PDST_OSC1_CTRL1,            2)                           \
    VARIANT(PDST_OSC1_LEVEL,            3)                           \
    VARIANT(PDST_OSC2_LEVEL,            4)                           \
    VARIANT(PDST_NOISE_LEVEL,           5)                           \
    VARIANT(PDST_FILTER1_BALANCE,       6)                           \
    VARIANT(PDST_FILTER1_CUTOFF,        7)                           \
    VARIANT(PDST_FILTER1_RESONANCE,     8)                           \
    VARIANT(PDST_FILTER2_CUTOFF,        9)                           \
    VARIANT(PDST_DRIVE_WS_DEPTH,        10)                          \
    VARIANT(PDST_AMP_LEVEL,             11)                          \
    VARIANT(PDST_AMP_PANPOT,            12)                          \
    VARIANT(PDST_LFO1_FREQ,             13)                          \
    VARIANT(PDST_LFO2_FREQ,             14)                          \
    VARIANT(PDST_PORTAMENTO,            15)                          \
    VARIANT(PDST_OSC1_CTRL2,            16)                          \
    VARIANT(PDST_FILTER1_EG1_INTENSITY, 17)                          \
    VARIANT(PDST_FILTER1_KEY_TRACK,     18)                          \
    VARIANT(PDST_FILTER2_RESONANCE,     19)                          \
    VARIANT(PDST_FILTER2_EG1_INTENSITY, 20)                          \
    VARIANT(PDST_FILTER2_KEY_TRACK,     21)                          \
    VARIANT(PDST_EG1_ATTACK,            22)                          \
    VARIANT(PDST_EG1_DECAY,             23)                          \
    VARIANT(PDST_EG1_SUSTAIN,           24)                          \
    VARIANT(PDST_EG1_RELEASE,           25)                          \
    VARIANT(PDST_EG2_ATTACK,            26)                          \
    VARIANT(PDST_EG2_DECAY,             27)                          \
    VARIANT(PDST_EG2_SUSTAIN,           28)                          \
    VARIANT(PDST_EG2_RELEASE,           29)                          \
    VARIANT(PDST_EG3_ATTACK,            30)                          \
    VARIANT(PDST_EG3_DECAY,             31)                          \
    VARIANT(PDST_EG3_SUSTAIN,           32)                          \
    VARIANT(PDST_EG3_RELEASE,           33)                          \
    VARIANT(PDST_PATCH1_INTENSITY,      34)                          \
    VARIANT(PDST_PATCH2_INTENSITY,      35)                          \
    VARIANT(PDST_PATCH3_INTENSITY,      36)                          \
    VARIANT(PDST_PATCH4_INTENSITY,      37)                          \
    VARIANT(PDST_PATCH5_INTENSITY,      38)                          \
    VARIANT(PDST_PATCH6_INTENSITY,      39)                          \

enum PatchDestination {
    PATCH_DST_ENUM(ENUM_VARIANT)
};


struct EnumElem {
    int value;
    const char* name;
};

struct EnumArr {
    EnumElem arr[MAX_ENUM_ELEMS];
    size_t size;
};

#define ENUM_ELEM_VARIANT(NAME, VALUE) EnumElem{ NAME, #NAME "###" #NAME },
#define ENUM_ARR(ENUM)                                               \
{                                                                    \
  .arr = {                                                           \
      ENUM(ENUM_ELEM_VARIANT)                                        \
  },                                                                 \
  .size = (0 ENUM(PLUS_ONE_VARIANT)),                                \
}                                                                    \

