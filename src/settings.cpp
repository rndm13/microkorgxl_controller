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

bool filter_type_balance_eq(FilterTypeBalance a, FilterTypeBalance b) {
    a = filter_type_balance_normalize(a);
    b = filter_type_balance_normalize(b);
    return a == b;
}

#define CONTROL_CHANGE_ENUM_STRING(NAME, VALUE) case NAME: return #NAME;

const char* control_change_name(ControlChange cc) {
    switch (cc) {
        CONTROL_CHANGE_ENUM(CONTROL_CHANGE_ENUM_STRING)
        default: assert(!"Unknown Control Change");
    }
    return "";
}
