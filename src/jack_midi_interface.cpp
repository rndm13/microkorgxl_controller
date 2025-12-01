#include "jack_midi_interface.hpp"

#define CC_VALUE_SIGN_BIT_OFFSET              13

#define CC_HEADER                             0xB0
#define CC_CHANNEL_MASK                       0x0F

#define EX_HEADER                             0xF042307E
#define EX_CHANNEL_MASK                       0x00000F00

#define EX_PROGRAM_WRITE_REQUEST_CODE         0x11

#define EX_PROGRAM_DUMP_REQUEST_CODE          0x1C
#define EX_PROGRAM_DUMP_CODE                  0x4C

#define EX_CUR_PROGRAM_DUMP_REQUEST_CODE      0x10
#define EX_CUR_PROGRAM_DUMP_CODE              0x40

#define EX_STATUS_WRITE_COMPLETED             0x21
#define EX_STATUS_WRITE_ERROR                 0x22
#define EX_STATUS_DATA_LOAD_COMPLETED         0x23
#define EX_STATUS_DATA_LOAD_ERROR             0x24
#define EX_STATUS_DATA_FORMAT_ERROR           0x26

#define EX_CC_CODE                            0x41

#define EX_END                                0xF7

#define PUSH_BYTES(X) this->push_bytes(reinterpret_cast<uint8_t*>(&(X)), sizeof(X));

static uint16_t encode_i16(uint16_t val);
static void debug_dump_bytes(const uint8_t *bytes, size_t size);

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
                out_buf, i,
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

const jack_midi_data_t* JACKMidi::handle_received_program(
        Program* cur_program,
        const jack_midi_data_t* buffer,
        ssize_t size) {
    Log(LogLevel::Debug, "Handling received program");
    for (size_t i = 0; i < size; i++) {
        if (buffer[i] == EX_END) {
            Log(LogLevel::Debug, "SYNTH, program_buffer_size: %zu", this->program_buffer_size);

            this->program_buffer_size = convert_from_midi_data(this->program_buffer, this->program_buffer_size);

            switch (this->state) {
                case JMS_DOWNLOADING_CUR_PROGRAM_DATA:
                    if (this->program_buffer_size != PROGRAM_SERIALIZED_SIZE) {
                        Log(LogLevel::Error, "Wrong program size %d", this->program_buffer_size);
                        break;
                    }

                    program_deserialize(cur_program, this->program_buffer);
                    break;
                case JMS_DOWNLOADING_PROGRAM_DATA:
                    // TODO:
                    break;
                case JMS_DOWNLOADING_GLOBAL_DATA:
                    // TODO:
                    break;
                default:
                    break;
            }

            this->state = JMS_RUNNING;
            // memset(this->program_buffer, 0, this->program_buffer_size);
            this->program_buffer_size = 0;

            return &buffer[i + 1];
        }

        this->program_buffer[this->program_buffer_size++] = buffer[i];
    }

    return NULL;
}

const jack_midi_data_t* JACKMidi::handle_received_event_ex(const jack_midi_data_t* buffer, ssize_t size) {
    uint8_t code = 0;

    Log(LogLevel::Debug, "Handling exclusive events");

    if (size <= 0) {
        return NULL;
    }

    code = buffer[0];
    buffer += 1;
    size -= 1;

    switch (code) {
        case EX_STATUS_WRITE_COMPLETED:
            Log(LogLevel::Info, "SYNTH: Write completed");
            break;
        case EX_STATUS_DATA_LOAD_COMPLETED:
            Log(LogLevel::Info, "SYNTH: Data load completed");
            break;
        case EX_STATUS_WRITE_ERROR:
            Log(LogLevel::Error, "SYNTH: Write error");
            break;
        case EX_STATUS_DATA_LOAD_ERROR:
            Log(LogLevel::Error, "SYNTH: Data load error");
            break;
        case EX_STATUS_DATA_FORMAT_ERROR:
            Log(LogLevel::Error, "SYNTH: Data format error");
            break;
        case EX_CUR_PROGRAM_DUMP_CODE:
            memset(this->program_buffer, 0, this->program_buffer_size);
            this->program_buffer_size = 0;
            this->state = JMS_DOWNLOADING_CUR_PROGRAM_DATA;
            break;
        case EX_PROGRAM_DUMP_CODE:
            break;
    }

    // Eat additional EOX byte
    switch (code) {
        case EX_STATUS_WRITE_COMPLETED:
        case EX_STATUS_DATA_LOAD_COMPLETED:
        case EX_STATUS_WRITE_ERROR:
        case EX_STATUS_DATA_LOAD_ERROR:
        case EX_STATUS_DATA_FORMAT_ERROR:
            buffer += 1;
            size -= 1;
            break;
        case EX_CUR_PROGRAM_DUMP_CODE:
            break;
        case EX_PROGRAM_DUMP_CODE:
            break;
    }

    return buffer;
}

const jack_midi_data_t* JACKMidi::handle_received_event(
        Program* cur_program,
        const jack_midi_data_t* buffer,
        ssize_t size) {
    switch (this->state) {
        case JMS_STOPPED:
        case JMS_RUNNING:
        case JMS_UPLOADING_PROGRAM_DATA:
            break;
        case JMS_DOWNLOADING_PROGRAM_DATA:
        case JMS_DOWNLOADING_CUR_PROGRAM_DATA:
        case JMS_DOWNLOADING_GLOBAL_DATA:
            return this->handle_received_program(cur_program, buffer, size);
            break;
    }

    uint32_t ex_header = 0;
    if (size >= sizeof(ex_header)) {
        memcpy(&ex_header, buffer, sizeof(ex_header));
        ex_header = htobe32(ex_header);
        if (ex_header == EX_HEADER) {
            buffer += sizeof(ex_header);
            size -= sizeof(ex_header);
            return handle_received_event_ex(buffer, size);
        }
    }

    return NULL;
}

bool JACKMidi::handle_received_data(Program* cur_program) {
    if (this->recv_buffer_size <= 0) {
        return true;
    }

    // TODO: refactor this to instead use events from JACK
    const jack_midi_data_t* cur_event = this->recv_buffer;
    ssize_t cur_size = this->recv_buffer_size;
    while (cur_event != NULL) {
        assert(cur_event <= this->recv_buffer + this->recv_buffer_size);
        if (cur_event == this->recv_buffer + this->recv_buffer_size) {
            // Reached the end
            break;
        }

        cur_size = this->recv_buffer_size - (cur_event - this->recv_buffer);
        cur_event = handle_received_event(cur_program, cur_event, cur_size);
    }

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

bool JACKMidi::send_cur_program_dump(const Program* cur_program) {
    memset(this->program_buffer, 0, sizeof(this->program_buffer));

    program_serialize(cur_program, this->program_buffer);
    this->program_buffer_size = PROGRAM_SERIALIZED_SIZE;

    this->program_buffer_size = convert_to_midi_data(this->program_buffer, this->program_buffer_size);

    if (this->program_buffer_size != PROGRAM_MIDI_SERIALIZED_SIZE) {
        Log(LogLevel::Error, "Wrong program midi buffer size: %d", this->program_buffer_size);
        return false;
    }

    uint32_t header = htobe32(EX_HEADER);
    uint8_t code = EX_CUR_PROGRAM_DUMP_CODE;
    uint8_t end = EX_END;

    PUSH_BYTES(header);
    PUSH_BYTES(code);

    this->push_bytes(this->program_buffer, PROGRAM_DUMP_SEND_SIZE);

    PUSH_BYTES(end);

    this->push_event();

    return true;
}

bool JACKMidi::send_program_write_req(uint8_t dst_program) {
    dst_program = 0x7F & dst_program;

    uint32_t header = htobe32(EX_HEADER);
    PUSH_BYTES(header);

    uint8_t code = EX_PROGRAM_WRITE_REQUEST_CODE;
    PUSH_BYTES(code);

    PUSH_BYTES(dst_program);

    dst_program = 0;
    PUSH_BYTES(dst_program);

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

bool JACKMidi::send_control_change_ex(ParamEx param, int16_t val) {
    assert(param.param_id < 1 << 14);
    assert(param.param_subid < 1 << 14);
    assert(val < 1 << 14);

    uint32_t header = htobe32(EX_HEADER);
    uint8_t code = EX_CC_CODE;
    param.param_id = encode_i16(param.param_id);
    param.param_subid = encode_i16(param.param_subid);
    val = encode_i16(val);
    uint8_t end = EX_END;

    PUSH_BYTES(header);
    PUSH_BYTES(code);
    PUSH_BYTES(param.param_id);
    PUSH_BYTES(param.param_subid);
    PUSH_BYTES(val);
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

    this->state = JMS_RUNNING;

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
    this->state = JMS_STOPPED;

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

    send_queue[this->send_queue_size++] = {
        .size = event_size,
        .buffer = event_data,
    };
}

void JACKMidi::push_bytes(const uint8_t *bytes, size_t size) {
    memcpy(this->send_buffer + this->send_buffer_size, bytes, size);
    this->send_buffer_size += size;
}

static uint16_t encode_i16(uint16_t val) {
    bool sign_bit = val < 0;
    val |= sign_bit << CC_VALUE_SIGN_BIT_OFFSET;
    val = htole16(val);

    uint16_t result = 0;

    result |= 0x7F & val;
    result |= 0x7F00 & (val << 1);

    return result;
}

static void debug_dump_bytes(const uint8_t *bytes, size_t size) {
    char buffer[256] = {0};
    size_t buffer_len = 0;

    for (size_t i = 0; i < size; i++) {
        if (i % 16 == 0) {
            Log(LogLevel::Debug, buffer);
            printf("%s\n", buffer);

            memset(buffer, 0, sizeof(buffer));
            buffer_len = 0;
        }

        buffer_len += snprintf(
                buffer + buffer_len, sizeof(buffer),
                "%02hhx ", bytes[i]);
    }

    Log(LogLevel::Debug, buffer);
    printf("%s\n", buffer);
    printf("\n\n\n");
}
