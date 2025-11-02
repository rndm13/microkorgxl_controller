#include "jack_midi_interface.hpp"

#define CC_HEADER                             0xB0
#define CC_CHANNEL_MASK                       0x0F

#define EX_HEADER                             0xF042307E
#define EX_CHANNEL_MASK                       0x00000F00

#define EX_CUR_PROGRAM_DUMP_REQUEST_CODE      0x10
#define EX_CC_CODE                            0x41

#define EX_END                                0xF7

#define PUSH_BYTES(X) this->push_bytes(reinterpret_cast<uint8_t*>(&(X)), sizeof(X));

static uint16_t encode_u16(uint16_t val);
static void debug_dump_bytes(uint8_t *bytes, size_t size);

static int process(jack_nframes_t nframes, void *arg) {
    assert(arg != nullptr);
    JACKMidi *jack_midi = reinterpret_cast<JACKMidi *>(arg);

    // ------------------
    //       INPUT
    // ------------------

    void *in_buf = jack_port_get_buffer(jack_midi->jack_port_in, nframes);
    if (in_buf == NULL) {
        Log(LogLevel::Error, "Failed to get in port buffer\n");
        return 1;
    }

    size_t event_cnt = jack_midi_get_event_count(in_buf);
    for (size_t i = 0; i < event_cnt; i++) {
        jack_midi_event_t e;

        int result = jack_midi_event_get(&e, in_buf, i);
        if (result != 0) {
            Log(LogLevel::Error, "Failed to read MIDI event %d\n", i);
            return 1;
        }

        if (e.size == 1 && e.buffer[0] == 0xf8) {
            // Ignore status messages

            continue;
        }

        memcpy(jack_midi->recv_buffer + jack_midi->recv_buffer_size, e.buffer, e.size);
        jack_midi->recv_buffer_size += e.size;
    }

    jack_midi_clear_buffer(in_buf);

    // ------------------
    //       OUTPUT
    // ------------------

    void *out_buf = jack_port_get_buffer(jack_midi->jack_port_out, nframes);
    if (out_buf == NULL) {
        Log(LogLevel::Error, "Failed to get out port buffer\n");
        return 1;
    }

    jack_midi_clear_buffer(out_buf);

    for (size_t i = 0; i < jack_midi->send_queue_size; i++) {
        int result = jack_midi_event_write(
                out_buf, nframes,
                jack_midi->send_queue[i].buffer,
                jack_midi->send_queue[i].size);
        if (result != 0) {
            Log(LogLevel::Error, "Failed to write MIDI event\n");
            return 1;
        }
    }

    memset(jack_midi->send_queue, 0, sizeof(jack_midi->send_queue));
    jack_midi->send_queue_size = 0;

    memset(jack_midi->send_buffer, 0, sizeof(jack_midi->send_buffer));
    jack_midi->send_buffer_size = 0;

    return 0;
}

bool JACKMidi::handle_received_data() {
    if (this->recv_buffer_size <= 0) {
        return true;
    }

    Log(LogLevel::Debug, "RECEIVED BYTES:");
    debug_dump_bytes(this->recv_buffer, this->recv_buffer_size);

    memset(this->recv_buffer, 0, this->recv_buffer_size);
    this->recv_buffer_size = 0;

    return true;
}

bool JACKMidi::send_cur_program_dump_req() {
    uint32_t header = htobe32(EX_HEADER);
    PUSH_BYTES(header);

    uint8_t code = EX_CUR_PROGRAM_DUMP_REQUEST_CODE;
    PUSH_BYTES(code);

    uint8_t end = EX_END;
    PUSH_BYTES(end);

    this->push_event();

    return true;
}

bool JACKMidi::send_control_change(uint8_t param_id, uint8_t val) {
    assert(param_id < 128);
    assert(val < 128);

    uint8_t header = CC_HEADER;
    PUSH_BYTES(header);
    PUSH_BYTES(param_id);
    PUSH_BYTES(val);

    this->push_event();

    return true;
}

bool JACKMidi::send_control_change_ex(ParamEx param, uint16_t val) {
    assert(param.param_id < 1 << 14);
    assert(param.param_subid < 1 << 14);
    assert(val < 1 << 14);

    uint32_t header = htobe32(EX_HEADER);
    PUSH_BYTES(header);

    uint8_t code = EX_CC_CODE;
    PUSH_BYTES(code);

    param.param_id = encode_u16(param.param_id);
    param.param_subid = encode_u16(param.param_subid);
    val = encode_u16(val);

    PUSH_BYTES(param.param_id);
    PUSH_BYTES(param.param_subid);
    PUSH_BYTES(val);

    uint8_t end = EX_END;
    PUSH_BYTES(end);

    this->push_event();

    return true;
}

bool JACKMidi::init() {
    memset(this->send_buffer, 0, sizeof(this->send_buffer));
    this->send_buffer_size = 0;
    memset(this->recv_buffer, 0, sizeof(this->recv_buffer));
    this->recv_buffer_size = 0;

    this->jack = jack_client_open(JACK_CLIENT_NAME, JackNoStartServer, NULL);

    if (this->jack == NULL) {
        Log(LogLevel::Error, "Failed to open JACK client\n");
    }

    Log(LogLevel::Info, "Successfully opened JACK client\n");

    int err = jack_set_process_callback(this->jack, process, this);
    if (err != 0) {
        Log(LogLevel::Error, "Failed to set JACK process callback. Err: %d\n", err);
        goto err_client;
    }

    this->jack_port_in = jack_port_register(this->jack, "MIDI In", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, JACK_PORT_BUFFER_SIZE);
    if (this->jack_port_in == NULL) {
        Log(LogLevel::Error, "Failed to register JACK IN port\n");
        goto err_client;
    }

    this->jack_port_out = jack_port_register(this->jack, "MIDI Out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, JACK_PORT_BUFFER_SIZE);
    if (this->jack_port_out == NULL) {
        Log(LogLevel::Error, "Failed to register JACK OUT port\n");
        goto err_port_in;
    }

    Log(LogLevel::Info, "Successfully registered JACK port\n");

    err = jack_activate(this->jack);
    if (err != 0) {
        Log(LogLevel::Error, "Failed to activate jack. Err: %d\n", err);
        goto err_port_out;
    }

    Log(LogLevel::Info, "JACK MIDI controller successfully initialized");
    return true;
err_port_out:
    jack_port_unregister(this->jack, this->jack_port_out);
err_port_in:
    jack_port_unregister(this->jack, this->jack_port_in);
err_client:
    jack_client_close(this->jack);
err:
    return false;
}

void JACKMidi::deinit() {
    jack_port_unregister(this->jack, this->jack_port_out);
    jack_port_unregister(this->jack, this->jack_port_in);
    jack_client_close(this->jack);

    Log(LogLevel::Info, "JACK MIDI controller successfully deinitialized");
}

void JACKMidi::push_event() {
    jack_midi_data_t *event_data = NULL;
    size_t event_size = 0;

    if (this->send_queue_size > 0) {
        jack_midi_event_t prev_event = this->send_queue[this->send_queue_size - 1];
        event_data = prev_event.buffer + prev_event.size;
        // Subtracts from the total size bytes behind previous event and previous event size;
        event_size = this->send_buffer_size - (prev_event.buffer - this->send_buffer) - prev_event.size;
    } else {
        event_data = this->send_buffer;
        event_size = this->send_buffer_size;
    }

    Log(LogLevel::Debug, "WRITTEN BYTES:");
    debug_dump_bytes(event_data, event_size);

    send_queue[this->send_queue_size++] = {
        .size = event_size,
        .buffer = event_data,
    };
}

void JACKMidi::push_bytes(const uint8_t *bytes, size_t size) {
    memcpy(this->send_buffer + this->send_buffer_size, bytes, size);
    this->send_buffer_size += size;
}

static uint16_t encode_u16(uint16_t val) {
    val = htole16(val);
    uint16_t result = 0;

    result |= 0x7F & val;
    result |= 0x7F00 & (val << 1);

    return result;
}

static void debug_dump_bytes(uint8_t *bytes, size_t size) {
    char buffer[256] = {0};
    size_t buffer_len = 0;

    for (size_t i = 0; i < size; i++) {
        if (i % 4 == 0) {
            Log(LogLevel::Debug, buffer);
            memset(buffer, 0, sizeof(buffer));
            buffer_len = 0;
        }
        buffer_len += snprintf(
                buffer + buffer_len, sizeof(buffer),
                "%hhx ", bytes[i]);
    }
    Log(LogLevel::Debug, buffer);

}
