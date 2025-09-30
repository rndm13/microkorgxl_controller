#include "immapp/immapp.h"

#ifdef IMGUI_BUNDLE_WITH_IMPLOT
#include "implot/implot.h"
#endif

#include "hello_imgui/hello_imgui.h"
#include "imgui-knobs/imgui-knobs.h"

#include "stdio.h"
#include "stdint.h"

#include "assert.h"

using HelloImGui::Log;
using HelloImGui::LogLevel;

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
    virtual bool send_control_change(uint8_t param_id, uint8_t val) = 0;
};

struct DummyMidi : public MidiInterface {
    virtual bool send_control_change(uint8_t param_id, uint8_t val) {
        assert(param_id < 128);
        assert(val < 128);

        Log(LogLevel::Debug, "CC %d %d", param_id, val);
        return true;
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
    out_app.midi = new DummyMidi();

    if (out_app.midi == nullptr) {
        Log(LogLevel::Error, "Failed to allocate MIDI interface");

        return false;
    }

    Log(LogLevel::Error, "Successfully initialized Controller");

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
        printf("Failed to initialize controller\n");
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
