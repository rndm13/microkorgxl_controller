#pragma once

#include <stddef.h>

#define ARRAY_SIZE(X) (sizeof(X) / (sizeof(*(X))))

#define TIMBRE_1_ID      0x10
#define TIMBRE_2_ID      0x20

#define ENUM_VARIANT(NAME, VALUE) NAME = VALUE,
#define PLUS_ONE_VARIANT(...) + 1


#define FILTER1_TYPE_BAL_ENUM(VARIANT)                               \
    VARIANT(FTB_24LPF, 0)                                            \
    VARIANT(FTB_12LPF, 31)                                           \
    VARIANT(FTB_HPF, 63)                                             \
    VARIANT(FTB_BPF, 95)                                             \
    VARIANT(FTB_THRU, 127)                                           \

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

#define MAX_ENUM_ELEMS 16

struct EnumElem {
    int value;
    const char* name;
};
struct EnumArr {
    EnumElem arr[MAX_ENUM_ELEMS];
    size_t size;
};

#define ENUM_ELEM_VARIANT(NAME, VALUE) EnumElem{ NAME, #NAME },
#define ENUM_ARR(ENUM)                                               \
{                                                                    \
  .arr = {                                                           \
      ENUM(ENUM_ELEM_VARIANT)                                        \
  },                                                                 \
  .size = (0 ENUM(PLUS_ONE_VARIANT)),                                \
}                                                                    \

