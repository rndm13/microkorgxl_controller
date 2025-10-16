#pragma once

#include "midi_interface.hpp"

struct DummyMidi : public MidiInterface {
    virtual bool send_control_change(uint8_t param_id, uint8_t val) override;

    virtual bool initialize() override;
};
