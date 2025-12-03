#pragma once

#include "program.hpp"
#include "midi_interface.hpp"
#include "imgui.h"

enum AppFlags {
    AF_NONE = 0,
    AF_TIMBRE_PARAMS = 1 << 0,
};

struct App {
    int flags;

    Program program;
    Timbre* selected_timbre;
    int selected_timbre_idx;

    MidiInterface* midi;

    ImFont* regular_font;
};

App* g_app();
bool app_init(App* out_app);
void app_deinit(App* out_app);
void app_push_timbre_params();
void app_pop_timbre_params();
ParamEx app_timbre_ex(ParamEx param);
