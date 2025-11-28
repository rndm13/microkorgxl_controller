#include "immapp/immapp.h"
#include "implot/implot.h"

#include "hello_imgui/hello_imgui.h"
#include "imgui-knobs/imgui-knobs.h"

#include "stdint.h"
#include <math.h>
#include "assert.h"

#include "program.hpp"
#include "jack_midi_interface.hpp"
#include "settings.hpp"

using HelloImGui::Log;
using HelloImGui::LogLevel;

struct App {
    Program program;
    Timbre* selected_timbre;
    MidiInterface* midi;
};

App g_app = {};

bool app_init(App& out_app) {
    Log(LogLevel::Info, "Initializing app");

    out_app.program = {};
    out_app.midi = new JACKMidi();

    if (out_app.midi == nullptr) {
        Log(LogLevel::Error, "Failed to allocate MIDI interface");

        return false;
    }

    if (!out_app.midi->init()) {
        Log(LogLevel::Error, "Failed to initialize MIDI interface");

        return false;
    }

    out_app.selected_timbre = &out_app.program.timbre_1;

    Log(LogLevel::Info, "Successfully initialized app");

    return true;
}

void app_deinit(App& app) {
    Log(LogLevel::Info, "Deinitializing app");

    app.midi->deinit();

    Log(LogLevel::Info, "Successfully deinitialized app");
}

static ParamEx timbre_ex(ParamEx param) {
    param.param_id = 0xFF00 & param.param_id;

    int16_t timbre_id = TIMBRE_1_ID;
    if (g_app.selected_timbre == &g_app.program.timbre_2) {
        timbre_id = TIMBRE_2_ID;
    }

    param.param_id |= timbre_id;

    return param;
}

// TODO: change value parameter to be a pointer (to match parameter_enum)
void parameter_knob(int& value, ControlChange cc, const char* name = NULL) {
    if (name == NULL) {
        name = control_change_name(cc);
    }

    if (ImGuiKnobs::KnobInt(name, &value, 0, 127, 1, "%d", ImGuiKnobVariant_Tick)) {
        g_app.midi->send_control_change(cc, value);
    }
}

void parameter_knob(int& value, ParamEx param, const char* name) {
    if (ImGuiKnobs::KnobInt(name, &value, 0, 127, 1, "%d", ImGuiKnobVariant_Tick)) {
        g_app.midi->send_control_change_ex(param, value);
    }
}

struct EnumElem {
    int value;
    const char* name;
};

#define ENUM_ARR_VARIANT(NAME, VALUE) EnumElem{ NAME, #NAME },

void parameter_enum(int* value, ParamEx param, const char* name, const EnumElem* enum_arr, size_t enum_arr_size) {
    size_t value_idx = 0;

    for (ssize_t i = enum_arr_size - 1; i > 0; i--) {
        if (*value >= enum_arr[i].value) {
            *value = enum_arr[i].value;
            value_idx = i;
            break;
        }
    }

    if (ImGui::BeginCombo(name, enum_arr[value_idx].name)) {
        for (size_t i = 0; i < enum_arr_size; i++) {
            bool is_selected = value_idx == i;
            if (ImGui::Selectable(enum_arr[i].name, is_selected)) {
                *value = enum_arr[i].value;
                g_app.midi->send_control_change_ex(param, *value);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void filter_graph_gui(Filter& filter, size_t filter_idx) {
    float y[128] = {0};
    float cutoff = filter.cutoff;
    float resonance = filter.resonance;
    FilterTypeBalance type_bal = static_cast<FilterTypeBalance>(filter.type_bal);
    if (filter_idx >= 1) {
        switch (filter.type_bal) {
            case FTB2_12LPF:
                type_bal = FTB_12LPF;
                break;
            case FTB2_HPF:
                type_bal = FTB_HPF;
                break;
            case FTB2_BPF:
                type_bal = FTB_BPF;
                break;
            default:
                type_bal = FTB_THRU;
        }
    }

    ImVec2 size = {-1, 80};

    if (type_bal == FTB_THRU) {
        ImGui::Button("Disabled", size);
        return;
    }

    float res_div = 64;
    float min_res = 24;
    if (type_bal == FTB_24LPF) {
        min_res = 48;
        res_div = 48;
    }

    for (size_t x = 0; x < ARRAY_SIZE(y); x++) {
        float parab = resonance -
            powf((-static_cast<float>(x) + cutoff) * fmax(resonance, min_res) / res_div, 2);

        y[x] = parab;
        if (type_bal == FTB_HPF) {
            if (x > cutoff) {
                y[x] = fmax(0, parab);
            }
        }

        if (type_bal == FTB_24LPF || type_bal == FTB_12LPF) {
            if (x < cutoff) {
                y[x] = fmax(0, parab);
            }
        }
    }

    ImGui::PlotLines("##FILTER_PLOT", y, ARRAY_SIZE(y), 0, NULL, -128, 128, size);
}

void filter_gui(Filter& filter, const char* window_name, size_t filter_idx) {
    static const ParamEx params[][2] = {
        {{0, 0x31}, {0, 0x40}}, // Balance
        {{0, 0x32}, {0, 0x42}}, // Cutoff
        {{0, 0x33}, {0, 0x43}}, // Resonance
        {{0, 0x34}, {0, 0x44}}, // EG Intensity
        {{0, 0x35}, {0, 0x45}}, // Keyboard track
        {{0, 0x36}, {0, 0x46}}, // Velocity sensitivity
    };
    static const EnumElem filter1_enum_elems[] = {
        FILTER_TYPE_BAL_ENUM(ENUM_ARR_VARIANT)
    };
    static const EnumElem filter2_enum_elems[] = {
        FILTER2_TYPE_BAL_ENUM(ENUM_ARR_VARIANT)
    };

    size_t enum_size = ARRAY_SIZE(filter1_enum_elems);
    const EnumElem* enum_elems = filter1_enum_elems;
    if (filter_idx == 1) {
        enum_elems = filter2_enum_elems;
        enum_size = ARRAY_SIZE(filter2_enum_elems);
    }

    if (ImGui::Begin(window_name)) {
        filter_graph_gui(filter, filter_idx);

        parameter_enum(&filter.type_bal, timbre_ex(params[0][filter_idx]), "Balance Type###type_bal", enum_elems, enum_size);
        parameter_knob(filter.cutoff, timbre_ex(params[1][filter_idx]), "Cutoff###cutoff");
        ImGui::SameLine();
        parameter_knob(filter.resonance, timbre_ex(params[2][filter_idx]), "Resonance###resonance");
        ImGui::SameLine();
        parameter_knob(filter.eg1_int, timbre_ex(params[3][filter_idx]), "EG1 Intensity###eg1_int");
        ImGui::SameLine();
        parameter_knob(filter.key_trk, timbre_ex(params[4][filter_idx]), "Key tracking###key_trk");
        ImGui::SameLine();
        parameter_knob(filter.vel_sens, timbre_ex(params[5][filter_idx]), "Velocity sensitivity###vel_sens");
    }

    ImGui::End(); // Begin
}

void oscillator_gui(Oscillator& osc, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(osc.wave, CC_OSC1_WAVE);
        ImGui::SameLine();
        parameter_knob(osc.osc_mod, CC_OSC1_OSC_MOD);
        ImGui::SameLine();
        parameter_knob(osc.control_arr[0], CC_OSC1_C1);
        ImGui::SameLine();
        parameter_knob(osc.control_arr[1], CC_OSC1_C2);
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
        // TODO: Vertical slider, also add osc2
        parameter_knob(mix.osc_lvl[0], timbre_ex({0, 0x28}), "OSC1###osc1");
        ImGui::SameLine();
        parameter_knob(mix.osc_lvl[1], timbre_ex({0, 0x29}), "OSC2###osc2");
        ImGui::SameLine();
        parameter_knob(mix.noise_lvl, timbre_ex({0, 0x2A}), "NOISE###noise");
    }

    ImGui::End(); // Begin
}

void amp_gui(Amp& amp, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        // TODO: More amp params...
        parameter_knob(amp.level, timbre_ex({0, 0x50}), "Level###level");
        ImGui::SameLine();
        parameter_knob(amp.depth, timbre_ex({0, 0x54}), "Depth###depth");
        ImGui::SameLine();
        parameter_knob(amp.panpot, timbre_ex({0, 0x55}), "Panpot###panpot");
    }

    ImGui::End(); // Begin
}

void eg_gui(EnvelopeGenerator& eg, const char* window_name, size_t idx) {
    const uint16_t param_mod = idx * 0x10;

    const ParamEx params[] = {
        {0, static_cast<uint16_t>(0x60 + param_mod)}, // Attack
        {0, static_cast<uint16_t>(0x61 + param_mod)}, // Decay
        {0, static_cast<uint16_t>(0x62 + param_mod)}, // Sustain
        {0, static_cast<uint16_t>(0x63 + param_mod)}, // Release
        {0, static_cast<uint16_t>(0x64 + param_mod)}, // Velocity sensitivity
    };

    if (ImGui::Begin(window_name)) {
        parameter_knob(eg.attack, timbre_ex(params[0]), "Attack###attack");
        ImGui::SameLine();
        parameter_knob(eg.decay, timbre_ex(params[1]), "Decay###decay");
        ImGui::SameLine();
        parameter_knob(eg.sustain, timbre_ex(params[2]), "Sustain###sustain");
        ImGui::SameLine();
        parameter_knob(eg.release, timbre_ex(params[3]), "Release###release");
        ImGui::SameLine();
        parameter_knob(eg.vel_sens, timbre_ex(params[4]), "Velocity Sensitivity###vel_sens");
    }

    ImGui::End(); // Begin
}

void lfo_gui(LFO& lfo, const char* window_name, size_t idx) {
    const uint16_t param_mod = idx * 0x10;

    const ParamEx params[] = {
        {0, static_cast<uint16_t>(0x90 + param_mod)}, // Wave type
        {0, static_cast<uint16_t>(0x92 + param_mod)}, // Frequency
        {0, static_cast<uint16_t>(0x93 + param_mod)}, // BPS sync
        {0, static_cast<uint16_t>(0x94 + param_mod)}, // Key sync
        {0, static_cast<uint16_t>(0x96 + param_mod)}, // Note sync
    };

    if (ImGui::Begin(window_name)) {
        parameter_knob(lfo.wave, timbre_ex(params[0]), "Wave###wave");
        ImGui::SameLine();
        parameter_knob(lfo.freq, timbre_ex(params[1]), "Frequency###freq");
    }

    ImGui::End(); // Begin
}

void patch_gui(Timbre& timbre, const char* window_name) {
    // TODO: Patches window / cool UI
}

void equalizer_gui(Equalizer& eq, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(eq.lo_gain, CC_EQ_LO_GAIN);
        ImGui::SameLine();
        parameter_knob(eq.hi_gain, CC_EQ_HI_GAIN);
    }

    ImGui::End(); // Begin
}

void effect_gui(Program& program, const char* window_name) {
    // TODO: Effects window
}

void timbre_gui(Timbre& timbre) {
    filter_gui(timbre.filter_arr[0], "Filter 1###filter1", 0);
    filter_gui(timbre.filter_arr[1], "Filter 2###filter2", 1);

    oscillator_gui(timbre.osc_1, "Oscillator 1");
    unison_gui(timbre.unison, "Unison");

    mixer_gui(timbre.mixer, "Mixer");
    equalizer_gui(timbre.eq, "Equalizer");
    amp_gui(timbre.amp, "Amp");

    eg_gui(timbre.eg_arr[0], "Envelope generator 1", 0);
    eg_gui(timbre.eg_arr[1], "Envelope generator 2", 1);
    eg_gui(timbre.eg_arr[2], "Envelope generator 3", 2);

    lfo_gui(timbre.lfo_arr[0], "Low Frequency Oscillator 1", 0);
    lfo_gui(timbre.lfo_arr[1], "Low Frequency Oscillator 2", 1);

    patch_gui(timbre, "Patches");

}

void program_gui(Program& program) {
    if (ImGui::Begin("Program###program")) {
        if (ImGui::InputText("Name", program.name, ARRAY_SIZE(program.name), ImGuiInputTextFlags_EnterReturnsTrue)) {
            for (uint16_t i = 0; i < ARRAY_SIZE(program.name) - 1; i++) {
                g_app.midi->send_control_change_ex({0, i}, program.name[i]);
            }
        }
    }

    effect_gui(program, "Effects");

    ImGui::End(); // Begin
}

void app_gui() {
    program_gui(g_app.program);
    timbre_gui(*g_app.selected_timbre);

    if (ImGui::Begin("Logs")) {
        HelloImGui::LogGui();
    }

    ImGui::End(); // Begin

#ifndef NDEBUG
    ImGui::ShowDemoWindow();
#endif
}

void app_menu_bar_gui() {
    if (ImGui::BeginMenu("MIDI")) {
        if (ImGui::MenuItem("Request Current Program Dump")) {
            bool ok = g_app.midi->send_cur_program_dump_req();
            if (!ok) {
                Log(LogLevel::Error, "Failed to send current program dump request");
            }
        }

        ImGui::EndMenu();
    }
}

void app_pre_frame() {
    bool ok = g_app.midi->handle_received_data();
    if (!ok) {
        Log(LogLevel::Error, "Failed to handle received data");
    }
}

int main(int, char *[]) {
    bool ok = app_init(g_app);
    if (!ok) {
        Log(LogLevel::Error, "Failed to initialize controller\n");
        return 1;
    }

#ifdef ASSETS_LOCATION
    HelloImGui::SetAssetsFolder(ASSETS_LOCATION);
#endif

    HelloImGui::RunnerParams runnerParams;
    runnerParams.imGuiWindowParams.showMenuBar = true;
    runnerParams.imGuiWindowParams.showStatus_Fps = true;
    runnerParams.callbacks.ShowGui = app_gui;
    runnerParams.callbacks.PreNewFrame = app_pre_frame;
    runnerParams.callbacks.ShowMenus = app_menu_bar_gui;
    runnerParams.fpsIdling.enableIdling = false;

    runnerParams.appWindowParams.windowTitle = "MicroKORG XL Controller";
    runnerParams.appWindowParams.windowGeometry = { .size = {600, 800} };

    ImmApp::AddOnsParams addOnsParams;
    addOnsParams.withImplot = true;

    ImmApp::Run(runnerParams, addOnsParams);

    app_deinit(g_app);

    return 0;
}
