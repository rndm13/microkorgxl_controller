#include "immapp/immapp.h"

#ifdef IMGUI_BUNDLE_WITH_IMPLOT
#include "implot/implot.h"
#endif

#include "hello_imgui/hello_imgui.h"
#include "imgui-knobs/imgui-knobs.h"

#include <jack/midiport.h>
#include <jack/jack.h>

#include "stdio.h"
#include "stdint.h"

#include "assert.h"

using HelloImGui::Log;
using HelloImGui::LogLevel;

#define JACK_CLIENT_NAME "MicroKORG XL Controller"
#define JACK_PORT_BUFFER_SIZE 4096
#define DATA_QUEUE_SIZE 4096

#define CONTROL_CHANGE_ENUM(VARIANT)                                 \
    VARIANT(CC_FILTER1_CUTOFF, 74)                                   \
    VARIANT(CC_FILTER1_RESO, 71)                                     

#define CONTROL_CHANGE_ENUM_VARIANT(NAME, VALUE) NAME = VALUE,
#define CONTROL_CHANGE_ENUM_STRING(NAME, VALUE) case NAME: return #NAME;

enum ControlChange {
    CONTROL_CHANGE_ENUM(CONTROL_CHANGE_ENUM_VARIANT)
};

const char* control_change_name(ControlChange cc) {
    switch (cc) {
        CONTROL_CHANGE_ENUM(CONTROL_CHANGE_ENUM_STRING)
        default: assert(!"Unknown Control Change");
    }
    return "";
}

struct MidiInterface {
    // TODO: Allow uint16_t values
    // TODO: Specify channels (needed for specifying timbres)
    virtual bool send_control_change(uint8_t param_id, uint8_t val) = 0;
    virtual bool initialize() = 0;
};

struct DummyMidi : public MidiInterface {
    virtual bool send_control_change(uint8_t param_id, uint8_t val) {
        assert(param_id < 128);
        assert(val < 128);

        Log(LogLevel::Debug, "CC %d %d", param_id, val);
        return true;
    }

    virtual bool initialize() {
        Log(LogLevel::Debug, "Dummy MIDI controller successfully initialized");
        return true;
    }
};

struct JACKMidi : public MidiInterface {
    jack_client_t* jack;
    jack_port_t* jack_port;
    jack_midi_data_t data_queue[DATA_QUEUE_SIZE];
    size_t data_queue_size;

    static int process(jack_nframes_t nframes, void* arg) {
        assert(arg != nullptr);
        JACKMidi *jack_midi = reinterpret_cast<JACKMidi*>(arg);
        if (jack_midi->data_queue_size <= 0) {
            return 0;
        }

        void* buf = jack_port_get_buffer(jack_midi->jack_port, nframes);
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

    virtual bool send_control_change(uint8_t param_id, uint8_t val) {
        assert(param_id < 128);
        assert(val < 128);

        this->data_queue[this->data_queue_size++] = 0xB0;
        this->data_queue[this->data_queue_size++] = param_id;
        this->data_queue[this->data_queue_size++] = val;
        
        return true;
    }

    virtual bool initialize() {
        memset(this->data_queue, 0, sizeof(this->data_queue));
        this->data_queue_size = 0;
        this->jack = jack_client_open(JACK_CLIENT_NAME, JackNoStartServer, NULL);

        if (this->jack == NULL) {
            Log(LogLevel::Error, "Failed to open JACK client\n");
        }

        Log(LogLevel::Info, "Successfully opened JACK client\n");

        int err = jack_set_process_callback(this->jack, JACKMidi::process, this);
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
};

struct Filter {
    int cutoff;
    int resonance;
};

struct Program {
    Filter filter_1;
};

struct App {
    Program program;
    MidiInterface* midi;
};

App g_app = {};

bool app_init(App& out_app) {
    Log(LogLevel::Info, "Initializing controller");

    out_app.program = {};
    out_app.midi = new JACKMidi();

    if (out_app.midi == nullptr) {
        Log(LogLevel::Error, "Failed to allocate MIDI interface");

        return false;
    }

    if (!out_app.midi->initialize()) {
        Log(LogLevel::Error, "Failed to initialize MIDI interface");

        return false;
    }

    Log(LogLevel::Error, "Successfully initialized controller");

    return true;
}

void parameter_knob(int& param, ControlChange cc) {
    if (ImGuiKnobs::KnobInt(control_change_name(cc), &param, 0, 127, 1, "%d", ImGuiKnobVariant_Tick)) {
        g_app.midi->send_control_change(cc, param);
    }
}

void filter_gui(Filter& filter, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(filter.cutoff, CC_FILTER1_CUTOFF);
        ImGui::SameLine();
        parameter_knob(filter.resonance, CC_FILTER1_RESO);
    }

    ImGui::End(); // Begin
}

void program_gui(Program& program) {
    filter_gui(program.filter_1, "Filter 1");
}

void app_gui() {
    program_gui(g_app.program);

#ifndef NDEBUG
    ImGui::ShowDemoWindow();
#endif
}

int main(int , char *[]) {
    bool succ = app_init(g_app);
    if (!succ) {
        Log(LogLevel::Error, "Failed to initialize controller\n");
        return 1;
    }

#ifdef ASSETS_LOCATION
    HelloImGui::SetAssetsFolder(ASSETS_LOCATION);
#endif

    HelloImGui::SimpleRunnerParams runnnerParams;
    runnnerParams.guiFunction = app_gui;
    runnnerParams.windowSize = {600, 800};

    ImmApp::AddOnsParams addOnsParams;
    addOnsParams.withImplot = true;

    ImmApp::Run(runnnerParams, addOnsParams);
    return 0;
}
