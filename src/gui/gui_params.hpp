#pragma once

#include "../midi_interface.hpp"
#include "../settings.hpp"

void parameter_slider(int* value, ParamEx param, const char* name, int min = 0, int max = 127);
void parameter_knob(int* value, ParamEx param, const char* name, int min = 0, int max = 127);
void parameter_input(int* value, ParamEx param, const char* name, int min = 0, int max = 127);
void parameter_checkbox(int* value, ParamEx param, const char* name);
void parameter_enum(int* value, ParamEx param, const char* name, const EnumArr* enum_arr);
