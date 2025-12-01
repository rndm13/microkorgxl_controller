#pragma once

#include "midi_interface.hpp"

#include <jack/jack.h>
#include <jack/midiport.h>

#include "hello_imgui/hello_imgui_logger.h"

#include <assert.h>
#include <atomic>

#include "program.hpp"

using HelloImGui::Log;
using HelloImGui::LogLevel;

#define JACK_CLIENT_NAME            "MicroKORG XL Controller"
#define JACK_PORT_BUFFER_SIZE       65535

#define DATA_QUEUE_ELEM_SIZE        512
#define DATA_QUEUE_SIZE             10

#define PROGRAM_BUFFER_SIZE         65535

enum JACKMidiState {
    JMS_STOPPED = 0,
    JMS_RUNNING = 1,
    JMS_DOWNLOADING_PROGRAM_DATA = 2,
    JMS_DOWNLOADING_CUR_PROGRAM_DATA = 3,
    JMS_DOWNLOADING_GLOBAL_DATA = 4,
    JMS_UPLOADING_PROGRAM_DATA = 5,
};

struct JACKMidi : public MidiInterface {
    jack_client_t *jack;
    jack_port_t *jack_port_in;
    jack_port_t *jack_port_out;

    JACKMidiState state;

    jack_midi_event_t send_queue[DATA_QUEUE_SIZE];
    std::atomic<size_t> send_queue_size;
    jack_midi_data_t send_buffer[DATA_QUEUE_SIZE * DATA_QUEUE_ELEM_SIZE];
    std::atomic<size_t> send_buffer_size;

    jack_midi_data_t recv_buffer[DATA_QUEUE_SIZE * DATA_QUEUE_ELEM_SIZE];
    size_t recv_buffer_size;

    jack_midi_data_t program_buffer[PROGRAM_BUFFER_SIZE];
    size_t program_buffer_size;

    bool handle_received_data(Program* cur_program) override;
    bool send_control_change(uint8_t param_id, uint8_t val) override;
    bool send_control_change_ex(ParamEx param, int16_t val) override;
    bool send_cur_program_dump_req() override;
    bool send_cur_program_dump(const Program* cur_program) override;
    bool send_program_write_req(uint8_t dst_program) override;
    bool init() override;
    void deinit() override;

    const jack_midi_data_t* handle_received_program(Program* cur_program, const jack_midi_data_t* buffer, ssize_t size);
    const jack_midi_data_t* handle_received_event_ex(const jack_midi_data_t* buffer, ssize_t size);
    const jack_midi_data_t* handle_received_event(Program* cur_program, const jack_midi_data_t* buffer, ssize_t size);

    void push_event();
    void push_events_if_needed();
    void push_bytes(const uint8_t *bytes, size_t size);
};
