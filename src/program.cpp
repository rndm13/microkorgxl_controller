#include "program.hpp"
#include "settings.hpp"

#include <assert.h>
#include <string.h>

static void filter_data_deserialize(Timbre* out_timbre, const SerializedFilterData* filter_data) {
    assert(out_timbre != nullptr);
    assert(filter_data != nullptr);

    out_timbre->filter_arr[0].type_bal = static_cast<FilterTypeBalance>(filter_data->filter1_bal);
    out_timbre->filter_arr[1].type_bal = static_cast<Filter2TypeBalance>(filter_data->filter2_type);

    for (size_t i = 0; i < ARRAY_SIZE(out_timbre->filter_arr); i++) {
        out_timbre->filter_arr[i].cutoff = filter_data->filter_arr[i].cutoff;
        out_timbre->filter_arr[i].resonance = filter_data->filter_arr[i].resonance;
        out_timbre->filter_arr[i].eg1_int = filter_data->filter_arr[i].eg1_int;
        out_timbre->filter_arr[i].key_trk = filter_data->filter_arr[i].key_trk;
        out_timbre->filter_arr[i].vel_sens = filter_data->filter_arr[i].vel_sens;
    }
}

static void unison_data_deserialize(Timbre* out_timbre, const SerializedProgramData* prog_data) {
    assert(out_timbre != nullptr);
    assert(prog_data != nullptr);

    // TODO: Add all of these to GUI
    out_timbre->unison.mode = prog_data->unison_voice;
    out_timbre->unison.detune = prog_data->unison_detune;
    out_timbre->unison.spread = prog_data->unison_spread;
    out_timbre->unison.voice_assign = prog_data->voice_assign;
}

static void osc_data_deserialize(Timbre* out_timbre, const SerializedProgramData* prog_data) {
    assert(out_timbre != nullptr);
    assert(prog_data != nullptr);

    out_timbre->osc_arr[0].wave = prog_data->osc1.wave_type;
    out_timbre->osc_arr[0].osc_mod = prog_data->osc1.osc_mod;
    for (size_t i = 0; i < ARRAY_SIZE(out_timbre->osc_arr[0].control_arr); i++) {
        out_timbre->osc_arr[0].control_arr[i] = prog_data->osc1.wave_ctrl[i];
    }
    out_timbre->osc_arr[0].pcm_dwgs_wave = prog_data->osc1.pcm_dwgs_wave_select;

    out_timbre->osc_arr[1].wave = prog_data->osc2.wave_type;
    out_timbre->osc_arr[1].osc_mod = prog_data->osc2.osc_mod;
    out_timbre->osc_arr[1].semitone = prog_data->osc2.semitone;
    out_timbre->osc_arr[1].tune = prog_data->osc2.tune;
}

static void mixer_data_deserialize(Timbre* out_timbre, const SerializedMixerData* mix_data) {
    assert(out_timbre != nullptr);
    assert(mix_data != nullptr);

    out_timbre->mixer.osc_lvl[0] = mix_data->osc_lvl_arr[0];
    out_timbre->mixer.osc_lvl[1] = mix_data->osc_lvl_arr[1];
    out_timbre->mixer.noise_lvl = mix_data->noise_lvl;
}

static void amp_data_deserialize(Timbre* out_timbre, const SerializedAmpData* amp_data) {
    assert(out_timbre != nullptr);
    assert(mix_data != nullptr);

    // TODO: Add all of these to GUI
    out_timbre->amp.lvl = amp_data->lvl;
    out_timbre->amp.drive_ws_pos = amp_data->drive_ws_pos;
    out_timbre->amp.wave_type = amp_data->wave_type;
    out_timbre->amp.depth = amp_data->depth;
    out_timbre->amp.panpot = amp_data->panpot;
    out_timbre->amp.key_trk = amp_data->key_trk;
    out_timbre->amp.punch_lvl = amp_data->punch_lvl;
}

static void eg_data_deserialize(Timbre* out_timbre, const SerializedProgramData* prog_data) {
    assert(out_timbre != nullptr);
    assert(mix_data != nullptr);

    for (size_t i = 0; i < ARRAY_SIZE(prog_data->eg_arr); i++) {
        out_timbre->eg_arr[i].attack = prog_data->eg_arr[i].attack;
        out_timbre->eg_arr[i].decay = prog_data->eg_arr[i].decay;
        out_timbre->eg_arr[i].sustain = prog_data->eg_arr[i].sustain;
        out_timbre->eg_arr[i].release = prog_data->eg_arr[i].release;
        out_timbre->eg_arr[i].vel_sens = prog_data->eg_arr[i].vel_sens;
    }
}

static void lfo_data_deserialize(Timbre* out_timbre, const SerializedProgramData* prog_data) {
    assert(out_timbre != nullptr);
    assert(mix_data != nullptr);

    // TODO: Add all of this to GUI
    for (size_t i = 0; i < ARRAY_SIZE(prog_data->lfo_arr); i++) {
        out_timbre->lfo_arr[i].wave = prog_data->lfo_arr[i].wave_type;
        out_timbre->lfo_arr[i].freq = prog_data->lfo_arr[i].freq;
        out_timbre->lfo_arr[i].key_sync = prog_data->lfo_arr[i].key_sync;
        out_timbre->lfo_arr[i].bpm_sync = prog_data->lfo_arr[i].bpm_sync;
        out_timbre->lfo_arr[i].note_sync = prog_data->lfo_arr[i].note_sync;
    }
}

static void patch_data_deserialize(Timbre* out_timbre, const SerializedProgramData* prog_data) {
    assert(out_timbre != nullptr);
    assert(mix_data != nullptr);

    // TODO: Add all of this to GUI
    for (size_t i = 0; i < ARRAY_SIZE(prog_data->patch_arr); i++) {
        out_timbre->patch_arr[i].src = prog_data->patch_arr[i].src;
        out_timbre->patch_arr[i].dst = prog_data->patch_arr[i].dst;
        out_timbre->patch_arr[i].intensity = prog_data->patch_arr[i].intensity;
    }
}

static void eq_data_deserialize(Timbre* out_timbre, const SerializedTimbre* timbre_data) {
    assert(out_timbre != nullptr);
    assert(mix_data != nullptr);

    // TODO: Add all of this to GUI
    out_timbre->eq.lo_gain = timbre_data->eq_lgain;
    out_timbre->eq.hi_gain = timbre_data->eq_hgain;
    out_timbre->eq.lo_freq = timbre_data->eq_lfreq;
    out_timbre->eq.hi_freq = timbre_data->eq_hfreq;
}

static void timbre_deserialize(Timbre* out_timbre, const SerializedTimbre* timbre_data) {
    assert(out_timbre != nullptr);
    assert(timbre_data != nullptr);

    filter_data_deserialize(out_timbre, &timbre_data->data.filter);
    osc_data_deserialize(out_timbre, &timbre_data->data);
    unison_data_deserialize(out_timbre, &timbre_data->data);
    mixer_data_deserialize(out_timbre, &timbre_data->data.mixer);
    amp_data_deserialize(out_timbre, &timbre_data->data.amp);
    eg_data_deserialize(out_timbre, &timbre_data->data);
    lfo_data_deserialize(out_timbre, &timbre_data->data);
    patch_data_deserialize(out_timbre, &timbre_data->data);
    eq_data_deserialize(out_timbre, timbre_data);
}

void program_deserialize(Program* out_prog, const uint8_t* data) {
    assert(out_prog != nullptr);
    assert(data != nullptr);

    const SerializedProgram *prog = reinterpret_cast<const SerializedProgram*>(data);

    strncpy(out_prog->name, prog->name, PROGRAM_NAME_LENGTH);
    for (size_t i = 0; i < ARRAY_SIZE(out_prog->timbre_arr); i++) {
        timbre_deserialize(&out_prog->timbre_arr[i], &prog->timbre_arr[i]);
    }
}

void program_serialize(const Program* prog, uint8_t* data) {
    assert(prog != nullptr);
    assert(data != nullptr);

    // TODO: Serialize program
}
