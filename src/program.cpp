#include "program.hpp"

#include <assert.h>
#include <string.h>

static void filter_data_deserialize(Timbre* out_timbre, const SerializedFilterData* filter_data) {
    assert(out_timbre != nullptr);
    assert(filter_data != nullptr);

    out_timbre->filter_1.balance = static_cast<FilterTypeBalance>(filter_data->filter1_bal);
    out_timbre->filter_1.cutoff = filter_data->filters[0].cutoff;
    out_timbre->filter_1.resonance = filter_data->filters[0].resonance;
}

static void timbre_deserialize(Timbre* out_timbre, const SerializedTimbre* timbre_data) {
    assert(out_timbre != nullptr);
    assert(timbre_data != nullptr);

    filter_data_deserialize(out_timbre, &timbre_data->data.filter);
}

void program_deserialize(Program* out_prog, const uint8_t* data) {
    assert(out_prog != nullptr);
    assert(data != nullptr);

    const SerializedProgram *prog = reinterpret_cast<const SerializedProgram*>(data);

    strncpy(out_prog->name, prog->name, PROGRAM_NAME_LENGTH);
    timbre_deserialize(&out_prog->timbre_arr[0], &prog->timbre_arr[0]);
    timbre_deserialize(&out_prog->timbre_arr[1], &prog->timbre_arr[1]);
}

void program_serialize(const Program* prog, uint8_t* data) {
    assert(prog != nullptr);
    assert(data != nullptr);

    // TODO:
}
