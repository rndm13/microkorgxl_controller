#include "immapp/immapp.h"
#include "implot/implot.h"

#include "hello_imgui/hello_imgui.h"
#include "imgui-knobs/imgui-knobs.h"

#include "stdint.h"

#include "assert.h"

#include "jack_midi_interface.hpp"

using HelloImGui::Log;
using HelloImGui::LogLevel;

#define CONTROL_CHANGE_ENUM(VARIANT)                                 \
    VARIANT(CC_FILTER1_CUTOFF, 74)                                   \
    VARIANT(CC_FILTER1_RESO, 71)

#define CONTROL_CHANGE_ENUM_VARIANT(NAME, VALUE) NAME = VALUE,
#define CONTROL_CHANGE_ENUM_STRING(NAME, VALUE) case NAME: return #NAME;

#define ARRAY_SIZE(X) (sizeof(X) / (sizeof(*(X))))

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

struct Filter {
    int cutoff;
    int resonance;
};

struct Program {
    char name[9];
    Filter filter_1;
};

struct App {
    Program program;
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

    Log(LogLevel::Info, "Successfully initialized app");

    return true;
}

void app_deinit(App& app) {
    Log(LogLevel::Info, "Deinitializing app");

    app.midi->deinit();

    Log(LogLevel::Info, "Successfully deinitialized app");
}

void parameter_knob(int& value, ControlChange cc) {
    if (ImGuiKnobs::KnobInt(control_change_name(cc), &value, 0, 127, 1, "%d", ImGuiKnobVariant_Tick)) {
        g_app.midi->send_control_change(cc, value);
    }
}

void parameter_knob_ex(int& value, ParamEx param, const char* name) {
    if (ImGuiKnobs::KnobInt(name, &value, 0, 127, 1, "%d", ImGuiKnobVariant_Tick)) {
        g_app.midi->send_control_change_ex(param, value);
    }
}

void filter_gui(Filter& filter, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob_ex(filter.cutoff, {0x11, 0x32}, control_change_name(CC_FILTER1_CUTOFF));
        ImGui::SameLine();
        parameter_knob(filter.resonance, CC_FILTER1_RESO);
    }

    ImGui::End(); // Begin
}

void program_gui(Program& program) {
    if (ImGui::Begin("Program")) {
        if (ImGui::InputText("Name", program.name, ARRAY_SIZE(program.name), ImGuiInputTextFlags_EnterReturnsTrue)) {
            for (uint16_t i = 0; i < ARRAY_SIZE(program.name) - 1; i++) {
                g_app.midi->send_control_change_ex({0, i}, program.name[i]);
            }
        }
    }

    ImGui::End(); // Begin

    filter_gui(program.filter_1, "Filter 1");
}

void app_gui() {
    program_gui(g_app.program);

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
