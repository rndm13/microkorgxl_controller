#include "settings.hpp"

#include <assert.h>

const char* control_change_name(ControlChange cc) {
    switch (cc) {
        CONTROL_CHANGE_ENUM(CONTROL_CHANGE_ENUM_STRING)
        default: assert(!"Unknown Control Change");
    }
    return "";
}
