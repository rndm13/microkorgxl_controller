#include "settings.hpp"

#include <assert.h>

#define FILTER_TYPE_BAL_ENUM_STRING(NAME, VALUE) \
    if (ftb >= (VALUE)) return #NAME;

const char* filter_type_balance_name(FilterTypeBalance ftb) {
    FILTER_TYPE_BAL_ENUM(FILTER_TYPE_BAL_ENUM_STRING);

    return "Unknown Filter Balance Type";
}

#define FILTER_TYPE_BAL_ENUM_NORMALIZE(NAME, VALUE) \
    if (ftb > (VALUE)) return NAME;

static FilterTypeBalance filter_type_balance_normalize(FilterTypeBalance ftb) {
    FILTER_TYPE_BAL_ENUM(FILTER_TYPE_BAL_ENUM_NORMALIZE);

    return FTB_THRU;
}

#define CONTROL_CHANGE_ENUM_STRING(NAME, VALUE) case NAME: return #NAME;

const char* control_change_name(ControlChange cc) {
    switch (cc) {
        CONTROL_CHANGE_ENUM(CONTROL_CHANGE_ENUM_STRING)
        default: assert(!"Unknown Control Change");
    }
    return "";
}
