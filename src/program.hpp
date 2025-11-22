#pragma once

#include <stdint.h>

#define PROGRAM_SERIALIZED_SIZE      342
#define PROGRAM_NAME_LENGTH          8

struct Filter {
    int balance;
    int cutoff;
    int resonance;
};

struct Timbre {
    // Filter 2 doesn't have THRU, and LPF24
    Filter filter_arr[2];
};

struct Program {
    char name[PROGRAM_NAME_LENGTH];
    Timbre timbre_1;
    Timbre timbre_2;
};

void program_deserialize(Program& out_prog, uint8_t* data);
void program_serialize(const Program& prog, uint8_t* data);
