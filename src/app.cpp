#include "app.hpp"

#include "jack_midi_interface.hpp"
#include "settings.hpp"

static App _app = {};

App* g_app() {
    return &_app;
}

bool app_init(App* out_app) {
    Log(LogLevel::Info, "Initializing app");

    *out_app = {};
    out_app->midi = new JACKMidi();

    if (out_app->midi == nullptr) {
        Log(LogLevel::Error, "Failed to allocate MIDI interface");

        return false;
    }

    if (!out_app->midi->init()) {
        Log(LogLevel::Error, "Failed to initialize MIDI interface");

        return false;
    }

    out_app->selected_timbre = &out_app->program.timbre_arr[out_app->selected_timbre_idx];

    Log(LogLevel::Info, "Successfully initialized app");

    return true;
}

void app_deinit(App* app) {
    Log(LogLevel::Info, "Deinitializing app");

    app->midi->deinit();

    Log(LogLevel::Info, "Successfully deinitialized app");
}

void app_push_timbre_params() {
    _app.flags |= AF_TIMBRE_PARAMS;
}

void app_pop_timbre_params() {
    _app.flags &= ~AF_TIMBRE_PARAMS;
}

ParamEx app_timbre_ex(ParamEx param) {
    param.param_id = param.param_id;

    int16_t timbre_id = TIMBRE_1_ID;
    if (_app.selected_timbre == &_app.program.timbre_arr[1]) {
        timbre_id = TIMBRE_2_ID;
    }

    param.param_id += timbre_id;

    return param;
}
