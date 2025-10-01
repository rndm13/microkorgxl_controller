#pragma once

#define ARRAY_SIZE(X) (sizeof(X) / (sizeof(*(X))))

#define TIMBRE_1_ID      0x11
#define TIMBRE_2_ID      0x22

#define FILTER_TYPE_BAL_ENUM(VARIANT)                                \
    VARIANT(FTB_24LPF, 0)                                            \
    VARIANT(FTB_12LPF, 31)                                           \
    VARIANT(FTB_HPF, 63)                                             \
    VARIANT(FTB_BPF, 95)                                             \
    VARIANT(FTB_THRU, 127)                                           \

#define FILTER2_TYPE_BAL_ENUM(VARIANT)                               \
    VARIANT(FTB2_12LPF, 0)                                           \
    VARIANT(FTB2_HPF, 1)                                             \
    VARIANT(FTB2_BPF, 2)                                             \

#define FILTER_TYPE_BAL_ENUM_VARIANT(NAME, VALUE) NAME = VALUE,

enum FilterTypeBalance {
    FILTER_TYPE_BAL_ENUM(FILTER_TYPE_BAL_ENUM_VARIANT)
};

enum Filter2TypeBalance {
    FILTER2_TYPE_BAL_ENUM(FILTER_TYPE_BAL_ENUM_VARIANT)
};

const char* filter_type_balance_name(FilterTypeBalance ftb);

#define CONTROL_CHANGE_ENUM(VARIANT)                                 \
    VARIANT(CC_FILTER1_CUTOFF, 74)                                   \
    VARIANT(CC_FILTER1_RESO, 71)                                     \
    VARIANT(CC_FILTER1_TYPE_BAL, 27)                                 \
    VARIANT(CC_FILTER1_ROUTING1, 26)                                 \
    VARIANT(CC_FILTER1_EG1_INT, 79)                                  \
    VARIANT(CC_OSC1_WAVE, 8)                                         \
    VARIANT(CC_OSC1_OSC_MODE, 9)                                     \
    VARIANT(CC_OSC1_OSC1C1, 15)                                      \
    VARIANT(CC_OSC1_OSC1C2, 17)                                      \
    VARIANT(CC_UNISON_MODE, 3)                                       \
    VARIANT(CC_MIXER_OSC1_LVL, 23)                                   \
    VARIANT(CC_MIXER_NOISE_LVL, 25)                                  \
    VARIANT(CC_AMP_LEVEL, 7)                                         \
    VARIANT(CC_AMP_PANPOT, 10)                                       \
    VARIANT(CC_DRIVE_WS_WS_DEPTH, 83)                                \
    VARIANT(CC_EG1_ATTACK, 85)                                       \
    VARIANT(CC_EG1_DECAY, 86)                                        \
    VARIANT(CC_EG1_SUSTAIN, 87)                                      \
    VARIANT(CC_EG1_RELEASE, 88)                                      \
    VARIANT(CC_LFO1_WAVE, 89)                                        \
    VARIANT(CC_LFO1_FREQ, 90)                                        \
    VARIANT(CC_PATCH1_INTENSTY, 103)                                 \
    VARIANT(CC_EQ_LO_GAIN, 110)                                      \
    VARIANT(CC_EQ_HI_GAIN, 109)                                      \
    VARIANT(CC_MST_FX1_DRY_WET, 115)                                 \
    VARIANT(CC_MST_FX1_CTRL_1, 12)                                   \
    VARIANT(CC_MST_FX1_CTRL_2, 112)                                  \

#define CONTROL_CHANGE_ENUM_VARIANT(NAME, VALUE) NAME = VALUE,

enum ControlChange {
    CONTROL_CHANGE_ENUM(CONTROL_CHANGE_ENUM_VARIANT)
};

const char* control_change_name(ControlChange cc);
