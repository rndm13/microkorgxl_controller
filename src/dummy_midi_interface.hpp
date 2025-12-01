#pragma once

#include "midi_interface.hpp"

struct DummyMidi : public MidiInterface {
    bool handle_received_data(Program* cur_program) override;
    bool send_control_change(uint8_t param_id, uint8_t val) override;
    bool send_control_change_ex(ParamEx param, int16_t val) override;
    bool send_cur_program_dump_req() override;
    bool send_cur_program_dump(const Program* cur_program) override;
    bool send_program_write_req(uint8_t dst_program) override;
    bool init() override;
    void deinit() override;
};
