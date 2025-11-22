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
    VARIANT(CC_FILTER1_RESO, 71)                                     \
    VARIANT(CC_FILTER1_TYPE_BAL, 27)                                 \
    VARIANT(CC_FILTER1_ROUTING1, 26)                                 \
    VARIANT(CC_FILTER1_EG1_INT, 79)                                  \
    VARIANT(CC_OSC1_WAVE, 8)                                         \
    VARIANT(CC_OSC1_OSC_MOD, 9)                                      \
    VARIANT(CC_OSC1_OSC1C1, 15)                                      \
    VARIANT(CC_OSC1_OSC1C2, 17)                                      \
    VARIANT(CC_UNISON_MODE, 3)                                       \
    VARIANT(CC_MIXER_OSC1_LVL, 23)                                   \
    VARIANT(CC_MIXER_NOISE_LVL, 25)                                  \
    VARIANT(CC_AMP_LEVEL, 7)                                         \
    VARIANT(CC_AMP_PANPOT, 10)                                       \
    VARIANT(CC_DRIVE_WS_WS_DEPTH, 83)                                \
    VARIANT(CC_EG1_ATTACK, 85)                                       \
    VARIANT(CC_EG1_DECAY, 86)                                        \
    VARIANT(CC_EG1_SUSTAIN, 87)                                      \
    VARIANT(CC_EG1_RELEASE, 88)                                      \
    VARIANT(CC_LFO1_WAVE, 89)                                        \
    VARIANT(CC_LFO1_FREQ, 90)                                        \
    VARIANT(CC_PATCH1_INTENSTY, 103)                                 \
    VARIANT(CC_EQ_LO_GAIN, 110)                                      \
    VARIANT(CC_EQ_HI_GAIN, 109)                                      \
    VARIANT(CC_MST_FX1_DRY_WET, 115)                                 \
    VARIANT(CC_MST_FX1_CTRL_1, 12)                                   \
    VARIANT(CC_MST_FX1_CTRL_2, 112)                                  \

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
    int type_bal;
    int routing1;
    int eg1_int;
    int key_trk;
};

struct Oscillator {
    int wave;
    int osc_mode;
    int osc1c1;
    int osc1c2;
};

struct Unison {
    int mode;
};

struct Mixer {
    int osc1_lvl;
    int noise_lvl;
};

struct Amp {
    int level;
    int panpot;
};

struct DriveWS {
    int ws_depth;
};

struct EnvelopeGenerator {
    int attack;
    int decay;
    int sustain;
    int release;
};

struct LFO {
    int wave;
    int freq;
};

struct Patch {
    int intensty;
};

struct Equalizer {
    int lo_gain;
    int hi_gain;
};

struct effect {
    int dry_wet;
    int ctrl_1;
    int ctrl_2;
};

struct Program {
    Filter filter_1;
    Oscillator osc_1;
    Unison unison;
    Mixer mixer;
    Amp amp;
    DriveWS drive_ws;
    EnvelopeGenerator eg_1;
    LFO lfo_1;
    Patch patch_1;
    Equalizer eq;
    effect effect_1;
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
        ImGui::SameLine();
        parameter_knob(filter.type_bal, CC_FILTER1_TYPE_BAL);
        ImGui::SameLine();
        parameter_knob(filter.routing1, CC_FILTER1_ROUTING1);
        ImGui::SameLine();
        parameter_knob(filter.eg1_int, CC_FILTER1_EG1_INT);
    }

    ImGui::End(); // Begin
}

void oscillator_gui(Oscillator& osc, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(osc.wave, CC_OSC1_WAVE);
        ImGui::SameLine();
        parameter_knob(osc.osc_mode, CC_OSC1_OSC_MODE);
        ImGui::SameLine();
        parameter_knob(osc.osc1c1, CC_OSC1_OSC1C1);
        ImGui::SameLine();
        parameter_knob(osc.osc1c2, CC_OSC1_OSC1C2);
    }

    ImGui::End(); // Begin
}

void unison_gui(Unison& unison, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(unison.mode, CC_UNISON_MODE);
    }

    ImGui::End(); // Begin
}

void mixer_gui(Mixer& mix, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(mix.osc1_lvl, CC_MIXER_OSC1_LVL);
        ImGui::SameLine();
        parameter_knob(mix.noise_lvl, CC_MIXER_NOISE_LVL);
    }

    ImGui::End(); // Begin
}

void amp_gui(Amp& amp, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(amp.level, CC_AMP_LEVEL);
        ImGui::SameLine();
        parameter_knob(amp.panpot, CC_AMP_PANPOT);
    }

    ImGui::End(); // Begin
}

void drive_gui(DriveWS& drive_ws, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(drive_ws.ws_depth, CC_DRIVE_WS_WS_DEPTH);
    }

    ImGui::End(); // Begin
}

void envelopegenerator_gui(EnvelopeGenerator& eg_1, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(eg_1.attack, CC_EG1_ATTACK);
        ImGui::SameLine();
        parameter_knob(eg_1.decay, CC_EG1_DECAY);
        ImGui::SameLine();
        parameter_knob(eg_1.sustain, CC_EG1_SUSTAIN);
        ImGui::SameLine();
        parameter_knob(eg_1.release, CC_EG1_RELEASE);
    }

    ImGui::End(); // Begin
}

void lfo_gui(LFO& lfo_1, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(lfo_1.wave, CC_LFO1_WAVE);
        ImGui::SameLine();
        parameter_knob(lfo_1.freq, CC_LFO1_FREQ);
    }

    ImGui::End(); // Begin
}

void patch_gui(Patch& patch_1, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(patch_1.intensty, CC_PATCH1_INTENSTY);
    }

    ImGui::End(); // Begin
}

void equalizer_gui(Equalizer& eq, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(eq.lo_gain, CC_EQ_LO_GAIN);
        ImGui::SameLine();
        parameter_knob(eq.hi_gain, CC_EQ_HI_GAIN);
    }

    ImGui::End(); // Begin
}

void effect_gui(Effect& effect_1, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(effect_1.dry_wet, CC_MST_FX1_DRY_WET);
        ImGui::SameLine();
        parameter_knob(effect_1.ctrl_1, CC_MST_FX1_CTRL_1);
        ImGui::SameLine();
        parameter_knob(effect_2.ctrl_2, CC_MST_FX1_CTRL_2);
    }

    ImGui::End(); // Begin
}

void program_gui(Program& program) {
    filter_gui(program.filter_1, "Filter 1");
    oscillator_gui(program.osc_1, "Oscillator 1");
    unison_gui(program.unison, "Unison");
    mixer_gui(program.mixer, "Mixer");
    amp_gui(program.amp, "Amp");
    drive_gui(program.drive_ws, "Drive/Ws");
    envelopegenerator_gui(program.eg_1, "Envelope generator");
    lfo_gui(program.lfo_1, "Low Frequency Oscillator");
    patch_gui(program.patch_1, "Patch 1");
    equalizer_gui(program.eq, "Equalizer");
    effect_gui(program.effect_1, "Master effect");
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
