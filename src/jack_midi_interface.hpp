#pragma once

#include "midi_interface.hpp"

#include <jack/jack.h>
#include <jack/midiport.h>

#include "hello_imgui/hello_imgui_logger.h"

#include <assert.h>

using HelloImGui::Log;
using HelloImGui::LogLevel;

#define JACK_CLIENT_NAME            "MicroKORG XL Controller"
#define JACK_PORT_BUFFER_SIZE       4096

#define DATA_QUEUE_ELEM_SIZE        512
#define DATA_QUEUE_SIZE             10

struct JACKMidi : public MidiInterface {
    jack_client_t *jack;
    jack_port_t *jack_port_in;
    jack_port_t *jack_port_out;

    jack_midi_event_t send_queue[DATA_QUEUE_SIZE];
    size_t send_queue_size;
    jack_midi_data_t send_buffer[DATA_QUEUE_SIZE * DATA_QUEUE_ELEM_SIZE];
    size_t send_buffer_size;

    jack_midi_data_t recv_buffer[DATA_QUEUE_SIZE * DATA_QUEUE_ELEM_SIZE];
    size_t recv_buffer_size;

    bool handle_received_data() override;
    void push_event();
    void push_bytes(const uint8_t *bytes, size_t size);
    bool send_control_change(uint8_t param_id, uint8_t val) override;
    bool send_control_change_ex(ParamEx param, uint16_t val) override;
    bool send_cur_program_dump_req() override;
    bool init() override;
    void deinit() override;
};
