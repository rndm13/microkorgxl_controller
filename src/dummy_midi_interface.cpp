#include "dummy_midi_interface.hpp"

#include <assert.h>
#include <hello_imgui/hello_imgui_logger.h>

using HelloImGui::Log;
using HelloImGui::LogLevel;

bool DummyMidi::handle_received_data(Program* cur_program) {
    return true;
}

bool DummyMidi::send_control_change(uint8_t param_id, uint8_t val) {
    assert(param_id < 128);
    assert(val < 128);

    Log(LogLevel::Debug, "CC %d %d", param_id, val);

    return true;
}

bool DummyMidi::send_control_change_ex(ParamEx param, int16_t val) {
    assert(param.param_id < 1 << 14);
    assert(param.param_subid < 1 << 14);
    assert(val < 1 << 14);

    Log(LogLevel::Debug, "CC EX %d:%d %d", param.param_id, param.param_subid, val);

    return true;
}

bool DummyMidi::init() {
    Log(LogLevel::Debug, "Dummy MIDI controller successfully initialized");

    return true;
}

void DummyMidi::deinit() {
    Log(LogLevel::Debug, "Dummy MIDI controller successfully deinitialized");
}

bool DummyMidi::send_cur_program_dump_req() {
    Log(LogLevel::Debug, "Current program dump request");

    return true;
}

bool DummyMidi::send_program_write_req(uint8_t dst_program) {
    Log(LogLevel::Debug, "Program write request");

    return true;
}

bool DummyMidi::send_cur_program_dump(const Program* cur_program) {
    Log(LogLevel::Debug, "Current program dump");

    return true;
}
