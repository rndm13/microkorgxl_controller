#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "program.hpp"

struct ParamEx {
    uint16_t param_id;
    uint16_t param_subid;
};

struct MidiInterface {
    // TODO: Allow uint16_t values
    // TODO: Specify channels (needed for specifying timbres)
    virtual bool handle_received_data(Program* cur_program) = 0;
    virtual bool send_control_change(uint8_t param_id, uint8_t val) = 0;
    virtual bool send_control_change_ex(ParamEx param, uint16_t val) = 0;
    virtual bool send_cur_program_dump_req() = 0;
    virtual bool init() = 0;
    virtual void deinit() = 0;
};

size_t convert_from_midi_data(uint8_t* buffer, ssize_t size);
size_t convert_to_midi_data(uint8_t* buffer, ssize_t size);
