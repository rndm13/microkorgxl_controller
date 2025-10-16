#pragma once

#include <stdint.h>

struct MidiInterface {
    // TODO: Allow uint16_t values
    // TODO: Specify channels (needed for specifying timbres)
    virtual bool send_control_change(uint8_t param_id, uint8_t val) = 0;
    virtual bool initialize() = 0;
};
