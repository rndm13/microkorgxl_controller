#pragma once

#include "midi_interface.hpp"

#include <jack/jack.h>
#include <jack/midiport.h>

#include "hello_imgui/hello_imgui_logger.h"

#include <assert.h>

using HelloImGui::Log;
using HelloImGui::LogLevel;

#define JACK_CLIENT_NAME "MicroKORG XL Controller"
#define JACK_PORT_BUFFER_SIZE 4096
#define DATA_QUEUE_SIZE 4096

struct JACKMidi : public MidiInterface {
    jack_client_t *jack;
    jack_port_t *jack_port;
    jack_midi_data_t data_queue[DATA_QUEUE_SIZE];
    size_t data_queue_size;

    virtual bool send_control_change(uint8_t param_id, uint8_t val) override;
    virtual bool init() override;
    virtual void deinit() override;
};
