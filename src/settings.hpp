#pragma once

#define ARRAY_SIZE(X) (sizeof(X) / (sizeof(*(X))))

#define CONTROL_CHANGE_ENUM(VARIANT)                                 \
    VARIANT(CC_FILTER1_CUTOFF, 74)                                   \
    VARIANT(CC_FILTER1_RESO, 71)

#define CONTROL_CHANGE_ENUM_VARIANT(NAME, VALUE) NAME = VALUE,
#define CONTROL_CHANGE_ENUM_STRING(NAME, VALUE) case NAME: return #NAME;

enum ControlChange {
    CONTROL_CHANGE_ENUM(CONTROL_CHANGE_ENUM_VARIANT)
};

const char* control_change_name(ControlChange cc);
