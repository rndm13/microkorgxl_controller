#include "jack_midi_interface.hpp"

static int process(jack_nframes_t nframes, void *arg) {
    assert(arg != nullptr);
    JACKMidi *jack_midi = reinterpret_cast<JACKMidi *>(arg);
    if (jack_midi->data_queue_size <= 0) {
        return 0;
    }

    void *buf = jack_port_get_buffer(jack_midi->jack_port, nframes);
    if (buf == NULL) {
        Log(LogLevel::Error, "Failed to get port buffer\n");
        return 1;
    }

    jack_midi_clear_buffer(buf);

    int result = jack_midi_event_write(buf, nframes, jack_midi->data_queue, jack_midi->data_queue_size);
    if (result != 0) {
        Log(LogLevel::Error, "Failed to write MIDI event\n");
        return 1;
    }

    memset(jack_midi->data_queue, 0, sizeof(jack_midi->data_queue));
    jack_midi->data_queue_size = 0;
    return 0;
}

bool JACKMidi::send_control_change(uint8_t param_id, uint8_t val) {
    assert(param_id < 128);
    assert(val < 128);

    this->data_queue[this->data_queue_size++] = 0xB0;
    this->data_queue[this->data_queue_size++] = param_id;
    this->data_queue[this->data_queue_size++] = val;

    return true;
}

bool JACKMidi::init() {
    memset(this->data_queue, 0, sizeof(this->data_queue));
    this->data_queue_size = 0;
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

    this->jack_port = jack_port_register(this->jack, "MIDI Out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, JACK_PORT_BUFFER_SIZE);
    if (this->jack_port == NULL) {
        Log(LogLevel::Error, "Failed to register JACK port\n");
        goto err_client;
    }

    Log(LogLevel::Info, "Successfully registered JACK port\n");

    err = jack_activate(this->jack);
    if (err != 0) {
        Log(LogLevel::Error, "Failed to activate jack. Err: %d\n", err);
        goto err_port;
    }

    Log(LogLevel::Info, "JACK MIDI controller successfully initialized");
    return true;
err_port:
    jack_port_unregister(this->jack, this->jack_port);
err_client:
    jack_client_close(this->jack);
err:
    return false;
}

void JACKMidi::deinit() {
    jack_port_unregister(this->jack, this->jack_port);
    jack_client_close(this->jack);

    Log(LogLevel::Info, "JACK MIDI controller successfully deinitialized");
}
