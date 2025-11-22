#include "immapp/immapp.h"
#include "implot/implot.h"

#include "hello_imgui/hello_imgui.h"
#include "imgui-knobs/imgui-knobs.h"

#include "stdint.h"

#include "assert.h"

#include "program.hpp"
#include "jack_midi_interface.hpp"
#include "settings.hpp"
#include <cmath>

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

    out_app.selected_timbre = &out_app.program.timbre_arr[1];

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
    if (g_app.selected_timbre == &g_app.program.timbre_arr[2]) {
        timbre_id = TIMBRE_2_ID;
    }

    param.param_id |= timbre_id;

    return param;
}

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

#define FILTER_TYPE_BAL_COMBO_VARIANT(NAME, VALUE)                             \
    {                                                                          \
        bool is_selected = filter_type_balance_eq(value, NAME);                \
        if (ImGui::Selectable(filter_type_balance_name(NAME), is_selected)) {  \
            value = NAME;                                                      \
            g_app.midi->send_control_change_ex(param, value);                  \
        }                                                                      \
        if (is_selected) {                                                     \
            ImGui::SetItemDefaultFocus();                                      \
        }                                                                      \
    }

void filter_balance_combo(FilterTypeBalance& value, ParamEx param, const char* name) {
    if (ImGui::BeginCombo(name, filter_type_balance_name(value))) {
        FILTER_TYPE_BAL_ENUM(FILTER_TYPE_BAL_COMBO_VARIANT);
        ImGui::EndCombo();
    }
}

void filter_graph_gui(Filter& filter) {
    float y[128] = {0};
    float cutoff = filter.cutoff;
    float resonance = filter.resonance;

    ImVec2 size = {-1, 80};
    if (filter.balance == FTB_THRU) {
        ImGui::Button("Disabled", size);
        return;
    }

    float res_div = 64;
    float min_res = 24;
    if (filter.balance == FTB_24LPF) {
        min_res = 48;
        res_div = 48;
    }

    for (size_t x = 0; x < ARRAY_SIZE(y); x++) {
        float parab = resonance -
            powf((-static_cast<float>(x) + cutoff) * fmax(resonance, min_res) / res_div, 2);

        y[x] = parab;
        if (filter.balance == FTB_HPF) {
            if (x > cutoff) {
                y[x] = fmax(0, parab);
            }
        }

        if (filter.balance == FTB_24LPF || filter.balance == FTB_12LPF) {
            if (x < cutoff) {
                y[x] = fmax(0, parab);
            }
        }
    }

    ImGui::PlotLines("##FILTER_PLOT", y, ARRAY_SIZE(y), 0, NULL, -128, 128, size);
}

void filter_gui(Filter& filter, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        filter_graph_gui(filter);

        filter_balance_combo(filter.balance, timbre_ex({0, 0x31}), control_change_name(CC_FILTER1_TYPE_BAL));
        parameter_knob(filter.cutoff, timbre_ex({0, 0x32}), control_change_name(CC_FILTER1_CUTOFF));
        ImGui::SameLine();
        parameter_knob(filter.resonance, timbre_ex({0, 0x33}), control_change_name(CC_FILTER1_RESO));
    }

    ImGui::End(); // Begin
}

void timbre_gui(Timbre& timbre) {
    filter_gui(timbre.filter_1, "Filter 1");
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
