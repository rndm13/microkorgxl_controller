#include "dummy_midi_interface.hpp"

#include <assert.h>
#include <hello_imgui/hello_imgui_logger.h>

using HelloImGui::Log;
using HelloImGui::LogLevel;

bool DummyMidi::send_control_change(uint8_t param_id, uint8_t val) {
    assert(param_id < 128);
    assert(val < 128);

    Log(LogLevel::Debug, "CC %d %d", param_id, val);

    return true;
}

bool DummyMidi::init() {
    Log(LogLevel::Debug, "Dummy MIDI controller successfully initialized");

    return true;
}

void DummyMidi::deinit() {
    Log(LogLevel::Debug, "Dummy MIDI controller successfully deinitialized");
}
