#include "program.hpp"
#include "settings.hpp"

#include <assert.h>
#include <endian.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static void filter_data_deserialize(Timbre* out_timbre, const SerializedFilterData* filter_data) {
    assert(out_timbre != nullptr);
    assert(filter_data != nullptr);

    out_timbre->filter_arr[0].type_bal = static_cast<Filter1TypeBalance>(filter_data->filter1_bal);
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

static void pitch_data_deserialize(Timbre* out_timbre, const SerializedProgramData* prog_data) {
    assert(out_timbre != nullptr);
    assert(prog_data != nullptr);

    // TODO: Add all of these to GUI
    out_timbre->pitch.analog_tuning = prog_data->analog_tuning;
    out_timbre->pitch.transpose = prog_data->transpose;
    out_timbre->pitch.detune = prog_data->detune;
    out_timbre->pitch.vibrato_int = prog_data->vibrato_int;
    out_timbre->pitch.bend_range = prog_data->bend_range;
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
    assert(amp_data != nullptr);

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
    assert(prog_data != nullptr);

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
    assert(prog_data != nullptr);

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
    assert(prog_data != nullptr);

    // TODO: Add all of this to GUI
    for (size_t i = 0; i < ARRAY_SIZE(prog_data->patch_arr); i++) {
        out_timbre->patch_arr[i].src = prog_data->patch_arr[i].src;
        out_timbre->patch_arr[i].dst = prog_data->patch_arr[i].dst;
        out_timbre->patch_arr[i].intensity = prog_data->patch_arr[i].intensity;
    }
}

static void eq_data_deserialize(Timbre* out_timbre, const SerializedTimbre* timbre_data) {
    assert(out_timbre != nullptr);
    assert(timbre_data != nullptr);

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
    pitch_data_deserialize(out_timbre, &timbre_data->data);
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

    out_prog->voice_mode = prog->voice_mode;
    out_prog->arp_timb_select = prog->arp_timb_select;
    out_prog->scale_key = prog->scale_key;
    out_prog->scale_type = prog->scale_type;
    out_prog->timbre2_midi_channel = prog->timbre2_midi_channel;
    out_prog->octave_sw = prog->octave_sw;

    out_prog->tempo = le16toh(prog->tempo);
}

static void filter_data_serialize(const Timbre* in_timbre, SerializedFilterData* filter_data) {
    assert(in_timbre != nullptr);
    assert(filter_data != nullptr);

    filter_data->filter1_bal = in_timbre->filter_arr[0].type_bal;
    filter_data->filter2_type = in_timbre->filter_arr[1].type_bal;

    for (size_t i = 0; i < ARRAY_SIZE(in_timbre->filter_arr); i++) {
        filter_data->filter_arr[i].cutoff = in_timbre->filter_arr[i].cutoff;
        filter_data->filter_arr[i].resonance = in_timbre->filter_arr[i].resonance;
        filter_data->filter_arr[i].eg1_int = in_timbre->filter_arr[i].eg1_int;
        filter_data->filter_arr[i].key_trk = in_timbre->filter_arr[i].key_trk;
        filter_data->filter_arr[i].vel_sens = in_timbre->filter_arr[i].vel_sens;
    }
}

static void unison_data_serialize(const Timbre* in_timbre, SerializedProgramData* prog_data) {
    assert(in_timbre != nullptr);
    assert(prog_data != nullptr);

    // TODO: Add all of these to GUI
    prog_data->unison_voice = in_timbre->unison.mode;
    prog_data->unison_detune = in_timbre->unison.detune;
    prog_data->unison_spread = in_timbre->unison.spread;
    prog_data->voice_assign = in_timbre->unison.voice_assign;
}

static void pitch_data_serialize(const Timbre* in_timbre, SerializedProgramData* prog_data) {
    assert(in_timbre != nullptr);
    assert(prog_data != nullptr);

    // TODO: Add all of these to GUI
    prog_data->analog_tuning = in_timbre->pitch.analog_tuning;
    prog_data->transpose = in_timbre->pitch.transpose;
    prog_data->detune = in_timbre->pitch.detune;
    prog_data->vibrato_int = in_timbre->pitch.vibrato_int;
    prog_data->bend_range = in_timbre->pitch.bend_range;
}

static void osc_data_serialize(const Timbre* in_timbre, SerializedProgramData* prog_data) {
    assert(in_timbre != nullptr);
    assert(prog_data != nullptr);

    prog_data->osc1.wave_type = in_timbre->osc_arr[0].wave;
    prog_data->osc1.osc_mod = in_timbre->osc_arr[0].osc_mod;
    for (size_t i = 0; i < ARRAY_SIZE(in_timbre->osc_arr[0].control_arr); i++) {
        prog_data->osc1.wave_ctrl[i] = in_timbre->osc_arr[0].control_arr[i];
    }
    prog_data->osc1.pcm_dwgs_wave_select = in_timbre->osc_arr[0].pcm_dwgs_wave;

    prog_data->osc2.wave_type = in_timbre->osc_arr[1].wave;
    prog_data->osc2.osc_mod = in_timbre->osc_arr[1].osc_mod;
    prog_data->osc2.semitone = in_timbre->osc_arr[1].semitone;
    prog_data->osc2.tune = in_timbre->osc_arr[1].tune;
}

static void mixer_data_serialize(const Timbre* in_timbre, SerializedMixerData* mix_data) {
    assert(in_timbre != nullptr);
    assert(mix_data != nullptr);

    mix_data->osc_lvl_arr[0] = in_timbre->mixer.osc_lvl[0];
    mix_data->osc_lvl_arr[1] = in_timbre->mixer.osc_lvl[1];
    mix_data->noise_lvl = in_timbre->mixer.noise_lvl;
}

static void amp_data_serialize(const Timbre* in_timbre, SerializedAmpData* amp_data) {
    assert(in_timbre != nullptr);
    assert(amp_data != nullptr);

    // TODO: Add all of these to GUI
    amp_data->lvl = in_timbre->amp.lvl;
    amp_data->drive_ws_pos = in_timbre->amp.drive_ws_pos;
    amp_data->wave_type = in_timbre->amp.wave_type;
    amp_data->depth = in_timbre->amp.depth;
    amp_data->panpot = in_timbre->amp.panpot;
    amp_data->key_trk = in_timbre->amp.key_trk;
    amp_data->punch_lvl = in_timbre->amp.punch_lvl;
}

static void eg_data_serialize(const Timbre* in_timbre, SerializedProgramData* prog_data) {
    assert(in_timbre != nullptr);
    assert(prog_data != nullptr);

    for (size_t i = 0; i < ARRAY_SIZE(prog_data->eg_arr); i++) {
        prog_data->eg_arr[i].attack = in_timbre->eg_arr[i].attack;
        prog_data->eg_arr[i].decay = in_timbre->eg_arr[i].decay;
        prog_data->eg_arr[i].sustain = in_timbre->eg_arr[i].sustain;
        prog_data->eg_arr[i].release = in_timbre->eg_arr[i].release;
        prog_data->eg_arr[i].vel_sens = in_timbre->eg_arr[i].vel_sens;
    }
}

static void lfo_data_serialize(const Timbre* in_timbre, SerializedProgramData* prog_data) {
    assert(in_timbre != nullptr);
    assert(prog_data != nullptr);

    // TODO: Add all of this to GUI
    for (size_t i = 0; i < ARRAY_SIZE(prog_data->lfo_arr); i++) {
        prog_data->lfo_arr[i].wave_type = in_timbre->lfo_arr[i].wave;
        prog_data->lfo_arr[i].freq = in_timbre->lfo_arr[i].freq;
        prog_data->lfo_arr[i].key_sync = in_timbre->lfo_arr[i].key_sync;
        prog_data->lfo_arr[i].bpm_sync = in_timbre->lfo_arr[i].bpm_sync;
        prog_data->lfo_arr[i].note_sync = in_timbre->lfo_arr[i].note_sync;
    }
}

static void patch_data_serialize(const Timbre* in_timbre, SerializedProgramData* prog_data) {
    assert(in_timbre != nullptr);
    assert(prog_data != nullptr);

    // TODO: Add all of this to GUI
    for (size_t i = 0; i < ARRAY_SIZE(prog_data->patch_arr); i++) {
        prog_data->patch_arr[i].src = in_timbre->patch_arr[i].src;
        prog_data->patch_arr[i].dst = in_timbre->patch_arr[i].dst;
        prog_data->patch_arr[i].intensity = in_timbre->patch_arr[i].intensity;
    }
}

static void eq_data_serialize(const Timbre* in_timbre, SerializedTimbre* timbre_data) {
    assert(in_timbre != nullptr);
    assert(timbre_data != nullptr);

    // TODO: Add all of this to GUI
    timbre_data->eq_lgain = in_timbre->eq.lo_gain;
    timbre_data->eq_hgain = in_timbre->eq.hi_gain;
    timbre_data->eq_lfreq = in_timbre->eq.lo_freq;
    timbre_data->eq_hfreq = in_timbre->eq.hi_freq;
}

static void timbre_serialize(const Timbre* in_timbre, SerializedTimbre* timbre_data) {
    assert(in_timbre != nullptr);
    assert(timbre_data != nullptr);

    filter_data_serialize(in_timbre, &timbre_data->data.filter);
    osc_data_serialize(in_timbre, &timbre_data->data);
    unison_data_serialize(in_timbre, &timbre_data->data);
    pitch_data_serialize(in_timbre, &timbre_data->data);
    mixer_data_serialize(in_timbre, &timbre_data->data.mixer);
    amp_data_serialize(in_timbre, &timbre_data->data.amp);
    eg_data_serialize(in_timbre, &timbre_data->data);
    lfo_data_serialize(in_timbre, &timbre_data->data);
    patch_data_serialize(in_timbre, &timbre_data->data);
    eq_data_serialize(in_timbre, timbre_data);
}

void program_serialize(const Program* in_prog, uint8_t* data) {
    assert(in_prog != nullptr);
    assert(data != nullptr);

    SerializedProgram *prog = reinterpret_cast<SerializedProgram*>(data);

    strncpy(prog->name, in_prog->name, PROGRAM_NAME_LENGTH);
    for (size_t i = 0; i < ARRAY_SIZE(in_prog->timbre_arr); i++) {
        timbre_serialize(&in_prog->timbre_arr[i], &prog->timbre_arr[i]);
    }

    prog->voice_mode = in_prog->voice_mode;
    prog->arp_timb_select = in_prog->arp_timb_select;
    prog->scale_key = in_prog->scale_key;
    prog->scale_type = in_prog->scale_type;
    prog->timbre2_midi_channel = in_prog->timbre2_midi_channel;
    prog->octave_sw = in_prog->octave_sw;

    prog->tempo = htole16(in_prog->tempo);
}

int program_save(const Program* prog, const char* filepath) {
    FileSerializedProgram file_prog = {};
    FILE *out_file = NULL;
    int rc = 0;

    memcpy(file_prog.header, FILE_PROGRAM_HEADER, sizeof(file_prog.header));

    program_serialize(prog, reinterpret_cast<uint8_t*>(&file_prog.prog));

    memset(file_prog.footer, 0xFF, sizeof(file_prog.footer));
    memset(file_prog.weird, 0xA0, sizeof(file_prog.weird));
    memset(file_prog.pad_zero, 0x00, sizeof(file_prog.pad_zero));
    memset(file_prog.pad_255, 0xFF, sizeof(file_prog.pad_255));

    out_file = fopen(filepath, "wb+");
    if (NULL == out_file) {
        rc = -1;
        goto cleanup;
    }

    fwrite(&file_prog, sizeof(file_prog), 1, out_file);

cleanup:
    fclose(out_file);

    return rc;
}

int program_open(Program* prog, const char* filepath) {
    FileSerializedProgram file_prog = {};
    FILE *in_file = NULL;
    size_t bytes_read = 0;
    int rc = 0;

    in_file = fopen(filepath, "rb");
    if (NULL == in_file) {
        rc = -1;
        goto cleanup;
    }

    bytes_read = fread(&file_prog, 1, sizeof(file_prog), in_file);
    if (bytes_read < sizeof(file_prog)) {
        rc = -1;
        goto cleanup;
    }

    if (0 != strncmp(
                reinterpret_cast<const char*>(file_prog.header),
                FILE_PROGRAM_STR_HEADER, strlen(FILE_PROGRAM_STR_HEADER))) {
        rc = -1;
        errno = EINVAL;
        goto cleanup;
    }

    program_deserialize(prog, reinterpret_cast<uint8_t*>(&file_prog.prog));

cleanup:
    fclose(in_file);

    return rc;
}
