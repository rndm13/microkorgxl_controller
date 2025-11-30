#define IMGUI_DEFINE_MATH_OPERATORS
#include "immapp/immapp.h"
#include "implot/implot.h"

#include "hello_imgui/hello_imgui.h"
#include "imgui-knobs/imgui-knobs.h"
#include "portable_file_dialogs/portable_file_dialogs.h"

#include "stdint.h"
#include "math.h"
#include "assert.h"
#include "stdio.h"

#include "program.hpp"
#include "jack_midi_interface.hpp"
#include "settings.hpp"

using HelloImGui::Log;
using HelloImGui::LogLevel;

#define GRAPH_SIZE ImVec2{320, 80}

#define EG_PARAM_NUM 4

float lerp(float a, float b, float f) {
    return a * (1.0 - f) + (b * f);
}

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

App g_app = {};

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

static ParamEx timbre_ex(ParamEx param) {
    param.param_id = param.param_id;

    int16_t timbre_id = TIMBRE_1_ID;
    if (g_app.selected_timbre == &g_app.program.timbre_arr[1]) {
        timbre_id = TIMBRE_2_ID;
    }

    param.param_id += timbre_id;

    return param;
}

static void push_timbre_params() {
    g_app.flags |= AF_TIMBRE_PARAMS;
}

static void pop_timbre_params() {
    g_app.flags &= ~AF_TIMBRE_PARAMS;
}

void parameter_slider(int* value, ParamEx param, const char* name, int min = 0, int max = 127) {
    if (g_app.flags & AF_TIMBRE_PARAMS) {
        param = timbre_ex(param);
    }

    *value = std::clamp(*value, min, max);

    if (ImGui::SliderScalar(name, ImGuiDataType_S32, value, &min, &max, "%d")) {
        g_app.midi->send_control_change_ex(param, *value);
    }
}

void parameter_knob(int* value, ParamEx param, const char* name, int min = 0, int max = 127) {
    if (g_app.flags & AF_TIMBRE_PARAMS) {
        param = timbre_ex(param);
    }

    *value = std::clamp(*value, min, max);

    if (ImGuiKnobs::KnobInt(name, value, min, max, 1, "%d", ImGuiKnobVariant_Tick)) {
        g_app.midi->send_control_change_ex(param, *value);
    }
}

void parameter_checkbox(int* value, ParamEx param, const char* name) {
    if (g_app.flags & AF_TIMBRE_PARAMS) {
        param = timbre_ex(param);
    }

    bool bval = *value;
    if (ImGui::Checkbox(name, &bval)) {
        *value = bval;
        g_app.midi->send_control_change_ex(param, *value);
    }
}

void parameter_enum(int* value, ParamEx param, const char* name, const EnumArr* enum_arr) {
    if (g_app.flags & AF_TIMBRE_PARAMS) {
        param = timbre_ex(param);
    }

    size_t value_idx = 0;

    for (ssize_t i = enum_arr->size - 1; i > 0; i--) {
        if (*value >= enum_arr->arr[i].value) {
            *value = enum_arr->arr[i].value;
            value_idx = i;
            break;
        }
    }

    if (ImGui::BeginCombo(name, enum_arr->arr[value_idx].name)) {
        for (size_t i = 0; i < enum_arr->size; i++) {
            bool is_selected = value_idx == i;
            if (ImGui::Selectable(enum_arr->arr[i].name, is_selected)) {
                *value = enum_arr->arr[i].value;
                g_app.midi->send_control_change_ex(param, *value);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void filter_graph_gui(Filter* filter, size_t idx) {
    float y[128] = {0};
    float cutoff = filter->cutoff;
    float resonance = filter->resonance;
    Filter1TypeBalance type_bal = static_cast<Filter1TypeBalance>(filter->type_bal);

    const float res_div = type_bal == FTB_24LPF ? 48 : 64;
    const float min_res = type_bal == FTB_24LPF ? 48 : 24;

    if (idx >= 1) {
        switch (filter->type_bal) {
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

    if (type_bal == FTB_THRU) {
        ImGui::Button("Disabled", GRAPH_SIZE);
        return;
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

    ImGui::PlotLines("###filter_graph", y, ARRAY_SIZE(y), 0, NULL, -128, 128, GRAPH_SIZE);
}

void filter_gui(Filter* filter, const char* window_name, size_t idx) {
    static const ParamEx params[][2] = {
        {{0x01, 0x31}, {0x01, 0x40}}, // Balance
        {{0x01, 0x32}, {0x01, 0x42}}, // Cutoff
        {{0x01, 0x33}, {0x01, 0x43}}, // Resonance
        {{0x01, 0x34}, {0x01, 0x44}}, // EG Intensity
        {{0x01, 0x35}, {0x01, 0x45}}, // Keyboard track
        {{0x01, 0x36}, {0x01, 0x46}}, // Velocity sensitivity
    };
    static const EnumArr type_bal_enum[] = {
        ENUM_ARR(FILTER1_TYPE_BAL_ENUM),
        ENUM_ARR(FILTER2_TYPE_BAL_ENUM)
    };

    if (ImGui::Begin(window_name)) {
        filter_graph_gui(filter, idx);

        parameter_enum(&filter->type_bal, params[0][idx], "Balance Type###type_bal", &type_bal_enum[idx]);
        parameter_knob(&filter->cutoff, params[1][idx], "Cutoff###cutoff");
        ImGui::SameLine();
        parameter_knob(&filter->resonance, params[2][idx], "Resonance###resonance");
        ImGui::SameLine();
        parameter_knob(&filter->eg1_int, params[3][idx], "EG1 Intensity###eg1_int");
        ImGui::SameLine();
        parameter_knob(&filter->key_trk, params[4][idx], "Key tracking###key_trk");
        ImGui::SameLine();
        parameter_knob(&filter->vel_sens, params[5][idx], "Velocity sensitivity###vel_sens");
    }

    ImGui::End(); // Begin
}

void oscillator_graph_gui(Oscillator* osc, size_t idx) {
    const ImVec2 graph_size = {-1, GRAPH_SIZE.y};
    float y[128] = {0};
    const char* button_str = nullptr;
    int wave = osc->wave;

    static bool first_run = true;
    static float saw[128] = {0};

    if (first_run) {
        first_run = false;

        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            saw[i] = -static_cast<float>(((i * 8) % 256) - 127);
        }
    }

    switch (wave) {
    case OSC1_WT_SAW:
        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            y[i] = saw[i];
        }
        break;
    case OSC1_WT_PULSE:
        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            y[i] = (saw[i] < 0 ? -1 : 1) * 127;
        }
        break;
    case OSC1_WT_SIN:
        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            y[i] = sin(i * 16.0f / 127.0f) * 127.0f;
        }
        break;
    case OSC1_WT_TRIANGLE:
        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            y[i] = abs(saw[i]) * 2 - 127;
        }
        break;
    case OSC1_WT_FORMANT:
        button_str = "FORMANT";
        break;
    case OSC1_WT_NOISE:
        button_str = "NOISE";
        break;
    case OSC1_WT_PCM_DWGS:
        button_str = "PCM/DWGS";
        break;
    case OSC1_WT_AUDIO_IN:
        button_str = "AUDIO IN";
        break;
    }

    if (button_str) {
        ImGui::Button(button_str, graph_size);
    } else {
        ImGui::PlotLines("###filter_graph", y, ARRAY_SIZE(y), 0, NULL, -128, 128, graph_size);
    }
}

void oscillator_gui(Oscillator* osc, const char* window_name, size_t idx) {
    static const ParamEx params[][2] = {
        {{0x01, 0x17}, {0x01, 0x20}}, // Wave Type
        {{0x01, 0x18}, {0x01, 0x21}}, // Mod
        {{0x01, 0x19}, {0x01, 0x22}}, // CTRL 1 / Semitone
        {{0x01, 0x1A}, {0x01, 0x23}}, // CTRL 2 / Tune
        {{0x01, 0x1B}, {0x01, 0xFF}}, // PCM DWGS
    };
    static const EnumArr wave_enum[] = {
        ENUM_ARR(OSC1_WAVE_TYPE_ENUM),
        ENUM_ARR(OSC2_WAVE_TYPE_ENUM)
    };
    static const EnumArr mod_enum[] = {
        ENUM_ARR(OSC1_OSC_MOD_ENUM),
        ENUM_ARR(OSC2_OSC_MOD_ENUM)
    };

    // TODO: wave select via a graph
    if (ImGui::Begin(window_name)) {
        oscillator_graph_gui(osc, idx);

        const float child_wcoeff = 0.5f + (idx * -0.1);
        const ImVec2 child_size = {ImGui::GetContentRegionAvail().x * child_wcoeff, 0};
        {
            ImGui::BeginChild("LeftSide###left", child_size);
            if (idx == 0) {
                parameter_knob(&osc->control_arr[0], params[2][idx], "Control 1");
                ImGui::SameLine();
                parameter_knob(&osc->control_arr[1], params[3][idx], "Control 2");
                ImGui::SameLine();
                parameter_knob(&osc->pcm_dwgs_wave, params[4][idx], "PCM/DWGS");
            } else {
                parameter_knob(&osc->semitone, params[2][idx], "Semitone");
                ImGui::SameLine();
                parameter_knob(&osc->tune, params[3][idx], "Tune");
            }
            ImGui::EndChild();
        }

        ImGui::SameLine();

        {
            ImGui::BeginChild("RightSide###right");
            parameter_enum(&osc->wave, params[0][idx], "Wave", &wave_enum[idx]);
            parameter_enum(&osc->osc_mod, params[1][idx], "Modulation", &mod_enum[idx]);
            ImGui::EndChild();
        }
    }

    ImGui::End(); // Begin
}

void unison_gui(Unison* unison, const char* window_name) {
    static const EnumArr uv_enum = ENUM_ARR(UNISON_VOICE_ENUM);
    static const EnumArr va_enum = ENUM_ARR(VOICE_ASSIGN_ENUM);

    if (ImGui::Begin(window_name)) {
        parameter_enum(&unison->mode, {0x01, 0x08}, "Unison Voice", &uv_enum);
        parameter_enum(&unison->voice_assign, {0x01, 0x0A}, "Spread", &va_enum);

        parameter_knob(&unison->detune, {0x01, 0x09}, "Detune", 0, 99);
        ImGui::SameLine();
        parameter_knob(&unison->spread, {0x01, 0x0A}, "Spread");
    }

    ImGui::End(); // Begin
}

void mixer_gui(Mixer* mix, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(&mix->osc_lvl[0], {0x01, 0x28}, "OSC1###osc1");
        ImGui::SameLine();
        parameter_knob(&mix->osc_lvl[1], {0x01, 0x29}, "OSC2###osc2");
        ImGui::SameLine();
        parameter_knob(&mix->noise_lvl, {0x01, 0x2A}, "NOISE###noise");
    }

    ImGui::End(); // Begin
}

void amp_gui(Amp* amp, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        // TODO: More amp params...
        parameter_knob(&amp->lvl, {0x01, 0x50}, "Level###level");
        ImGui::SameLine();
        parameter_knob(&amp->depth, {0x01, 0x54}, "Depth###depth");
        ImGui::SameLine();
        parameter_knob(&amp->panpot, {0x01, 0x55}, "Panpot###panpot");
    }

    ImGui::End(); // Begin
}

void eg_graph_gui(EnvelopeGenerator* eg, const ParamEx *params) {
    const int LINE_WIDTH = 2;
    const int GRAB_RADIUS = 5;
    const int SENSITIVITY = 1;

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImGuiIO& IO = ImGui::GetIO();

    const ImVec2 size = ImGui::GetContentRegionAvail();
    ImVec2 bb[2] = {
        ImGui::GetCursorScreenPos(),
        ImGui::GetCursorScreenPos() + size,
    };

    float attack = eg->attack / 127.0f / 4;
    float decay = eg->decay / 127.0f / 4;
    const float post_decay = 1.0f / 4;
    float sustain = eg->sustain / 127.0f;
    float release = eg->release / 127.0f / 4;

    ImVec2 points[6] = {
        {0, 0},

        {attack, 1},
        {attack + decay, sustain},
        {attack + decay + post_decay, sustain},
        {attack + decay + post_decay + release, 0},

        {1, 0},
    };

    // Grabbers
    ImGui::BeginChild("###eg", size, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_NoScrollbar);

    ImDrawList* DrawList = ImGui::GetWindowDrawList();

    ImVec2 orig_pos = ImGui::GetCursorScreenPos();
    for (int i = EG_PARAM_NUM - 1; i >= 0; i--) {
        size_t point_i = i + 1;
        ImGui::PushID(i);

        ImVec2 pos = ImVec2{points[point_i].x, 1 - points[point_i].y} * (bb[1] - bb[0]) + bb[0];
        ImGui::SetCursorScreenPos(pos - ImVec2(GRAB_RADIUS, GRAB_RADIUS));
        ImGui::InvisibleButton("###grabber", ImVec2(2 * GRAB_RADIUS, 2 * GRAB_RADIUS));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
            int dx = IO.MouseDelta.x / SENSITIVITY;
            int dy = IO.MouseDelta.y / SENSITIVITY;
            ParamEx param_ex = params[i];
            if (g_app.flags & AF_TIMBRE_PARAMS) {
                param_ex = timbre_ex(param_ex);
            }

            int* param_ptr = NULL;
            int change = dx;

            switch (i) {
                case 0:
                    param_ptr = &eg->attack;
                    break;
                case 1:
                    param_ptr = &eg->decay;
                    break;
                case 2:
                    param_ptr = &eg->sustain;
                    change = -dy;
                    break;
                case 3:
                    param_ptr = &eg->release;
                    break;
            }

            *param_ptr = std::clamp(*param_ptr + change, 0, 127);
            g_app.midi->send_control_change_ex(param_ex, *param_ptr);
        }

        ImGui::PopID();
    }
    ImGui::SetCursorScreenPos(orig_pos);

    // Drawing
    const ImColor c_line = style.Colors[ImGuiCol_PlotLines];
    const ImColor c_point = style.Colors[ImGuiCol_Text];
    for (int i = 0; i < ARRAY_SIZE(points) - 1; i++) {
        ImVec2 p1 = { points[i+0].x, 1 - points[i+0].y };
        ImVec2 p2 = { points[i+1].x, 1 - points[i+1].y };

        ImVec2 s = {p1.x * (bb[1].x - bb[0].x) + bb[0].x, p1.y * (bb[1].y - bb[0].y) + bb[0].y};
        ImVec2 e = {p2.x * (bb[1].x - bb[0].x) + bb[0].x, p2.y * (bb[1].y - bb[0].y) + bb[0].y};

        DrawList->AddLine(s, e, c_line, LINE_WIDTH);
    }

    for (int i = 1; i < ARRAY_SIZE(points) - 1; i++) {
        ImVec2 p1 = { points[i+0].x, 1 - points[i+0].y };
        ImVec2 s = {p1.x * (bb[1].x - bb[0].x) + bb[0].x, p1.y * (bb[1].y - bb[0].y) + bb[0].y};
        DrawList->AddCircleFilled(s, GRAB_RADIUS, c_point);
    }

    ImGui::EndChild();
}

void eg_gui(EnvelopeGenerator* eg, const char* window_name, size_t idx) {
    const uint16_t param_mod = idx * 0x10;

    const ParamEx params[] = {
        {0x01, static_cast<uint16_t>(0x60 + param_mod)}, // Attack
        {0x01, static_cast<uint16_t>(0x61 + param_mod)}, // Decay
        {0x01, static_cast<uint16_t>(0x62 + param_mod)}, // Sustain
        {0x01, static_cast<uint16_t>(0x63 + param_mod)}, // Release
        {0x01, static_cast<uint16_t>(0x64 + param_mod)}, // Velocity sensitivity
    };

    if (ImGui::Begin(window_name)) {
        parameter_slider(&eg->vel_sens, params[4], "Velocity Sensitivity###vel_sens");
        eg_graph_gui(eg, params);
    }

    ImGui::End(); // Begin
}

void lfo_graph_gui(LFO* lfo, size_t idx) {
    static const int rand_points[][2] = {
        {0,   0   },
        {10,  0   },
        {11,  127 },
        {25,  56  },
        {40,  -127},
        {67,  45  },
        {75,  -65 },
        {98,  127 },
        {105, 90  },
        {116, -10 },
        {127, 0   },
        {128, 0   }
    };
    static bool first_run = true;
    static float rand[128] = {0};
    static float saw[128] = {0};
    float y[128] = {0};
    static const int lfo2_wave_mod = 0x10;
    int wave = idx == 0 ? lfo->wave : lfo->wave | lfo2_wave_mod;

    if (first_run) {
        first_run = false;

        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            saw[i] = -static_cast<float>(((i * 8) % 256) - 127);
        }

        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            int j = 0;
            for (j = 0; j < ARRAY_SIZE(rand_points); j++) {
                if (i < rand_points[j + 1][0]) {
                    break;
                }
            }

            float f = float(i - rand_points[j][0]) / float(rand_points[j + 1][0] - rand_points[j][0]);
            rand[i] = lerp(rand_points[j][1], rand_points[j + 1][1], f);
        }
    }

    switch (wave) {
    case LFO1_W_SAW:
    case lfo2_wave_mod | LFO2_W_SAW:
        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            y[i] = saw[i];
        }
        break;
    case LFO1_W_SQUARE:
        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            y[i] = (saw[i] < 0 ? -1 : 1) * 127;
        }
        break;
    case lfo2_wave_mod | LFO2_W_SQUAREP:
        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            y[i] = (saw[i] <= 0 ? 0 : 1) * 127;
        }
        break;
    case lfo2_wave_mod | LFO2_W_SINE:
        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            y[i] = sin(i * 16.0f / 127.0f) * 127.0f;
        }
        break;
    case LFO1_W_TRIANGLE:
        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            y[i] = abs(saw[i]) * 2 - 127;
        }
        break;
    case lfo2_wave_mod | LFO2_W_S_AND_H:
    case LFO1_W_S_AND_H: {
        float sample = 0;
        const int sample_freq = 16;
        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            if (i % sample_freq == 0) {
                sample = rand[i];
            }
            y[i] = sample;
        }
        break;
    }
    case LFO1_W_RANDOM:
    case lfo2_wave_mod | LFO2_W_RANDOM:
        for (int i = 0; i < ARRAY_SIZE(y); i++) {
            y[i] = rand[i];
        }
        break;
    }

    ImGui::PlotLines("###lfo_graph", y, ARRAY_SIZE(y), 0, NULL, -128, 128, GRAPH_SIZE);
}

void lfo_gui(LFO* lfo, const char* window_name, size_t idx) {
    const uint16_t param_mod = idx * 0x10;

    const ParamEx params[] = {
        {0x01, static_cast<uint16_t>(0x90 + param_mod)}, // Wave type
        {0x01, static_cast<uint16_t>(0x92 + param_mod)}, // Frequency
        {0x01, static_cast<uint16_t>(0x93 + param_mod)}, // BPM sync
        {0x01, static_cast<uint16_t>(0x94 + param_mod)}, // Key sync
        {0x01, static_cast<uint16_t>(0x96 + param_mod)}, // Note sync
    };

    const EnumArr wave_enum[] = {
        ENUM_ARR(LFO1_WAVE_ENUM),
        ENUM_ARR(LFO2_WAVE_ENUM),
    };
    const EnumArr ks_enum = ENUM_ARR(LFO_KEY_SYNC_ENUM);

    if (ImGui::Begin(window_name)) {
        const ImVec2 size_l = {GRAPH_SIZE.x, 180};

        {
            ImGui::BeginChild("LeftSide###left", size_l);
            lfo_graph_gui(lfo, idx);
            ImGui::SetNextItemWidth(240);
            parameter_enum(&lfo->wave, params[0], "Wave###wave", &wave_enum[idx]);

            ImGui::SetNextItemWidth(240);
            parameter_enum(&lfo->key_sync, params[3], "Key Sync###key_sync", &ks_enum);
            parameter_checkbox(&lfo->bpm_sync, params[2], "BPM Sync###bpm_sync");
            ImGui::EndChild();
        }

        ImGui::SameLine();

        {
            ImGui::BeginChild("RightSide###right");
            parameter_knob(&lfo->freq, params[1], "Frequency###freq");
            parameter_knob(&lfo->note_sync, params[4], "Note Sync###note_sync", 0, 16);
            ImGui::EndChild();
        }
    }

    ImGui::End(); // Begin
}

void patch_gui(Timbre* timbre, const char* window_name) {
    // TODO: Patches window / cool UI
}

void equalizer_gui(Equalizer* eq, const char* window_name) {
    if (ImGui::Begin(window_name)) {
        parameter_knob(&eq->lo_freq, {0x09, 0x00}, "Low Frequency###lo_freq", 0, 33);
        ImGui::SameLine();
        parameter_knob(&eq->lo_gain, {0x09, 0x01}, "Low Gain###lo_gain", 0, 127); // ????
        parameter_knob(&eq->hi_freq, {0x09, 0x02}, "High Frequency###hi_freq", 0, 25);
        ImGui::SameLine();
        parameter_knob(&eq->hi_gain, {0x09, 0x03}, "High Gain###hi_gain", 0, 127); // ????
    }

    ImGui::End(); // Begin
}

void effect_gui(Program* program, const char* window_name) {
    // TODO: Effects window
}

void timbre_gui(Timbre* timbre) {
    push_timbre_params();

    filter_gui(&timbre->filter_arr[0], "Filter 1###filter1", 0);
    filter_gui(&timbre->filter_arr[1], "Filter 2###filter2", 1);

    oscillator_gui(&timbre->osc_arr[0], "Oscillator 1", 0);
    oscillator_gui(&timbre->osc_arr[1], "Oscillator 2", 1);
    unison_gui(&timbre->unison, "Unison");

    mixer_gui(&timbre->mixer, "Mixer");
    equalizer_gui(&timbre->eq, "Equalizer");
    amp_gui(&timbre->amp, "Amp");

    eg_gui(&timbre->eg_arr[0], "Envelope generator 1", 0);
    eg_gui(&timbre->eg_arr[1], "Envelope generator 2", 1);
    eg_gui(&timbre->eg_arr[2], "Envelope generator 3", 2);

    lfo_gui(&timbre->lfo_arr[0], "Low Frequency Oscillator 1", 0);
    lfo_gui(&timbre->lfo_arr[1], "Low Frequency Oscillator 2", 1);

    patch_gui(timbre, "Patches");

    pop_timbre_params();
}

void program_gui(Program* program) {
    if (ImGui::Begin("Program###program")) {
        ImGui::SetNextItemWidth(80);
        if (ImGui::InputText("Name###name", program->name, ARRAY_SIZE(program->name), ImGuiInputTextFlags_EnterReturnsTrue)) {
            for (uint16_t i = 0; i < ARRAY_SIZE(program->name) - 1; i++) {
                g_app.midi->send_control_change_ex({0x0, i}, program->name[i]);
            }
        }

        int16_t min = 20;
        int16_t max = 300;
        ImGui::SameLine();
        ImGui::SetNextItemWidth(160);
        if (ImGui::SliderScalar("Tempo###tempo", ImGuiDataType_S16, &program->tempo, &min, &max, "%d")) {
            g_app.midi->send_control_change_ex({0x60, 0}, program->tempo);
        }

        static const char* items[] = {"Timbre 1", "Timbre 2"};

        ImGui::SameLine();
        ImGui::SetNextItemWidth(160);
        if (ImGui::Combo("Timbre###timbre", &g_app.selected_timbre_idx, items, ARRAY_SIZE(items))) {
            g_app.selected_timbre = &g_app.program.timbre_arr[g_app.selected_timbre_idx];
        }
    }

    effect_gui(program, "Effects");

    ImGui::End(); // Begin
}

void app_gui() {
    ImGui::PushFont(g_app.regular_font);
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, 0);

    program_gui(&g_app.program);
    timbre_gui(g_app.selected_timbre);

    ImGui::PopStyleColor();
    ImGui::PopFont();

    if (ImGui::Begin("Logs")) {
        HelloImGui::LogGui();
    }
    ImGui::End(); // Begin

#ifndef NDEBUG
    ImGui::ShowDemoWindow();
#endif
}

void dialog_cur_program_save() {
    pfd::save_file save_file("Current program data", "", {"All files", "*", "MicroKORG XL Program", "*.mkxl_prog"}, pfd::opt::none);

    std::string result = save_file.result();
    if (result.size() <= 0) {
        return;
    }

    int rc = program_save(&g_app.program, result.c_str());
    if (rc < 0) {
        Log(LogLevel::Error, "Failed to save program to file: %s", strerror(errno));
        return;
    }

    Log(LogLevel::Info, "Successfully saved program file");
}

void dialog_program_open() {
    pfd::open_file open_file("Program data", "", {"All files", "*", "MicroKORG XL Program", "*.mkxl_prog"}, pfd::opt::none);

    std::vector<std::string> result = open_file.result();
    if (result.empty()) {
        return;
    }

    int rc = program_open(&g_app.program, result[0].c_str());
    if (rc < 0) {
        Log(LogLevel::Error, "Failed to open program file: %s", strerror(errno));
        return;
    }

    Log(LogLevel::Info, "Successfully opened program file");
}

void app_menu_bar_gui() {
    if (ImGui::BeginMenu("MIDI")) {
        if (ImGui::MenuItem("Request Current Program Dump")) {
            bool ok = g_app.midi->send_cur_program_dump_req();
            if (!ok) {
                Log(LogLevel::Error, "Failed to send current program dump request");
            }
        }

        if (ImGui::MenuItem("Send Current Program Dump")) {
            bool ok = g_app.midi->send_cur_program_dump(&g_app.program);
            if (!ok) {
                Log(LogLevel::Error, "Failed to send current program dump");
            }
        }

        if (ImGui::MenuItem("Write request")) {
            bool ok = g_app.midi->send_program_write_req(0);
            if (!ok) {
                Log(LogLevel::Error, "Failed to send program write request");
            }
        }

        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save current program")) {
            dialog_cur_program_save();
        }

        if (ImGui::MenuItem("Open program")) {
            dialog_program_open();
        }

        ImGui::EndMenu();
    }
}

void app_pre_frame() {
    bool ok = g_app.midi->handle_received_data(&g_app.program);
    if (!ok) {
        Log(LogLevel::Error, "Failed to handle received data");
    }
}

void load_fonts() {
    g_app.regular_font = HelloImGui::LoadFont("fonts/DroidSans.ttf", 12);
}

int main(int, char *[]) {
    bool ok = app_init(&g_app);
    if (!ok) {
        Log(LogLevel::Error, "Failed to initialize controller\n");
        return 1;
    }

#ifdef ASSETS_LOCATION
    HelloImGui::SetAssetsFolder(ASSETS_LOCATION);
#endif

    HelloImGui::RunnerParams runnerParams = {};
    runnerParams.imGuiWindowParams.showMenuBar = true;
    runnerParams.imGuiWindowParams.showStatus_Fps = true;

    runnerParams.callbacks.ShowGui = app_gui;
    runnerParams.callbacks.PreNewFrame = app_pre_frame;
    runnerParams.callbacks.ShowMenus = app_menu_bar_gui;
    runnerParams.callbacks.LoadAdditionalFonts = load_fonts;

    runnerParams.fpsIdling.enableIdling = false;

    runnerParams.appWindowParams.windowTitle = "MicroKORG XL Controller";
    runnerParams.appWindowParams.windowGeometry = { .size = {600, 800} };

    ImmApp::AddOnsParams addOnsParams;
    addOnsParams.withImplot = true;

    ImmApp::Run(runnerParams, addOnsParams);

    app_deinit(&g_app);

    return 0;
}
