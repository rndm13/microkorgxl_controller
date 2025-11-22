#pragma once

#include <stdint.h>

#include "settings.hpp"

#define PROGRAM_SERIALIZED_SIZE      342
#define PROGRAM_NAME_LENGTH          8

#define TIMBRE_NUM                   2

struct Filter {
    FilterTypeBalance balance;
    int cutoff;
    int resonance;
};

struct Timbre {
    Filter filter_1;
};

struct Program {
    char name[PROGRAM_NAME_LENGTH + 1];
    Timbre timbre_arr[TIMBRE_NUM];
};

void program_deserialize(Program& out_prog, uint8_t* data);
void program_serialize(const Program& prog, uint8_t* data);
