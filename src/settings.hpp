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

#define FILTER_TYPE_BAL_ENUM_VARIANT(NAME, VALUE) NAME = VALUE,

enum FilterTypeBalance {
    FILTER_TYPE_BAL_ENUM(FILTER_TYPE_BAL_ENUM_VARIANT)
};

const char* filter_type_balance_name(FilterTypeBalance ftb);

#define CONTROL_CHANGE_ENUM(VARIANT)                                 \
    VARIANT(CC_FILTER1_CUTOFF, 74)                                   \
    VARIANT(CC_FILTER1_RESO, 71)                                     \
    VARIANT(CC_FILTER1_TYPE_BAL, 27)                                 \

#define CONTROL_CHANGE_ENUM_VARIANT(NAME, VALUE) NAME = VALUE,

enum ControlChange {
    CONTROL_CHANGE_ENUM(CONTROL_CHANGE_ENUM_VARIANT)
};

const char* control_change_name(ControlChange cc);
