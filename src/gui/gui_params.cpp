#include "gui_params.hpp"

#include "../app.hpp"

#include <algorithm>
#include <math.h>

#include "imgui-knobs/imgui-knobs.h"

void parameter_slider(int* value, ParamEx param, const char* name, int min, int max) {
    if (g_app()->flags & AF_TIMBRE_PARAMS) {
        param = app_timbre_ex(param);
    }

    *value = std::clamp(*value, min, max);

    if (ImGui::SliderScalar(name, ImGuiDataType_S32, value, &min, &max, "%d")) {
        g_app()->midi->send_control_change_ex(param, *value);
    }
}

void parameter_knob(int* value, ParamEx param, const char* name, int min, int max) {
    if (g_app()->flags & AF_TIMBRE_PARAMS) {
        param = app_timbre_ex(param);
    }

    *value = std::clamp(*value, min, max);

    if (ImGuiKnobs::KnobInt(name, value, min, max, 1, "%d", ImGuiKnobVariant_Tick)) {
        g_app()->midi->send_control_change_ex(param, *value);
    }
}

void parameter_input(int* value, ParamEx param, const char* name) {
    if (g_app()->flags & AF_TIMBRE_PARAMS) {
        param = app_timbre_ex(param);
    }

    if (ImGui::InputScalar(name, ImGuiDataType_U16, value)) {
        g_app()->midi->send_control_change_ex(param, *value);
    }
}

void parameter_checkbox(int* value, ParamEx param, const char* name) {
    if (g_app()->flags & AF_TIMBRE_PARAMS) {
        param = app_timbre_ex(param);
    }

    bool bval = *value;
    if (ImGui::Checkbox(name, &bval)) {
        *value = bval;
        g_app()->midi->send_control_change_ex(param, *value);
    }
}

void parameter_enum(int* value, ParamEx param, const char* name, const EnumArr* enum_arr) {
    if (g_app()->flags & AF_TIMBRE_PARAMS) {
        param = app_timbre_ex(param);
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
                g_app()->midi->send_control_change_ex(param, *value);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}
