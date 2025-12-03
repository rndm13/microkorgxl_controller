#include "gui_tests.hpp"

#include "../src/settings.hpp"
#include "../src/app.hpp"

#include "hello_imgui/hello_imgui.h"

#include "imgui_test_engine/imgui_te_context.h"
#include "imgui_test_engine/imgui_te_engine.h"

#define ENUM_SEL(NAME) "**/###" #NAME

void register_tests() {
    ImGuiTestEngine* e = HelloImGui::GetImGuiTestEngine();

    static constexpr const char* root_sel = "//";

    static constexpr const char* win_filter1_sel = "//###filter1";
    static constexpr const char* win_filter2_sel = "//###filter2";
    static constexpr const char* filt_type_bal_sel = "/###type_bal";
    static constexpr const char* filt_cutoff_sel = "/###cutoff/###cutoff";
    static constexpr const char* filt_resonance_sel = "/###resonance/###resonance";
    static constexpr const char* filt_eg1_int_sel = "/###eg1_int/###eg1_int";
    static constexpr const char* filt_key_trk_sel = "/###key_trk/###key_trk";
    static constexpr const char* filt_vel_sens_sel = "/###vel_sens/###vel_sens";

    static constexpr const char* win_osc1_sel = "//###osc1";
    static constexpr const char* win_osc2_sel = "//###osc2";
    static constexpr const char* osc_wave_sel = "**/###wave";
    static constexpr const char* osc_mod_sel = "**/###mod";

    static constexpr const char* osc_control1_sel = "/###control1/###control1";
    static constexpr const char* osc_control2_sel = "/###control2/###control2";
    static constexpr const char* osc_pcm_dwgs_wave_sel = "/###pcm_dwgs_wave/###pcm_dwgs_wave";
    static constexpr const char* osc_semitone_sel = "/###semitone/###semitone";
    static constexpr const char* osc_tune_sel = "/###tune/###tune";

    static constexpr const char* win_eg1_sel = "//###eg1";
    static constexpr const char* win_eg2_sel = "//###eg2";
    static constexpr const char* win_eg3_sel = "//###eg3";

    static constexpr const char* eg_graph_sel = "/###eg";
    static constexpr const char* eg_attack_sel = "/$$0/###grabber";
    static constexpr const char* eg_decay_sel = "/$$1/###grabber";
    static constexpr const char* eg_sustain_sel = "/$$2/###grabber";
    static constexpr const char* eg_release_sel = "/$$3/###grabber";

    static constexpr const char* eg_vel_sens_sel = "/###vel_sens";

    auto enum_click = [](ImGuiTestContext* ctx, const char* win_sel, const char* enum_sel) {
        ctx->SetRef(win_sel);
        ctx->SetRef(enum_sel);

        ctx->ItemClick("");
        ctx->SetRef(root_sel);
    };

    auto enum_click_child = [](ImGuiTestContext* ctx, const char* win_sel, const char* child_sel, const char* enum_sel) {
        ctx->SetRef(win_sel);
        ctx->SetRef(ctx->WindowInfo(child_sel).Window);
        ctx->SetRef(enum_sel);

        ctx->ItemClick("");
        ctx->SetRef(root_sel);
    };

    ImGuiTest* filter1__balance_type = IM_REGISTER_TEST(e, "filter1", "balance_type");
    filter1__balance_type->TestFunc = [&](ImGuiTestContext* ctx) {
        enum_click(ctx, win_filter1_sel, filt_type_bal_sel);
        ctx->ItemClick(ENUM_SEL(FTB1_24LPF));
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].type_bal, FTB1_24LPF);

        enum_click(ctx, win_filter1_sel, filt_type_bal_sel);
        ctx->ItemClick(ENUM_SEL(FTB1_12LPF));
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].type_bal, FTB1_12LPF);

        enum_click(ctx, win_filter1_sel, filt_type_bal_sel);
        ctx->ItemClick(ENUM_SEL(FTB1_HPF));
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].type_bal, FTB1_HPF);

        enum_click(ctx, win_filter1_sel, filt_type_bal_sel);
        ctx->ItemClick(ENUM_SEL(FTB1_BPF));
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].type_bal, FTB1_BPF);

        enum_click(ctx, win_filter1_sel, filt_type_bal_sel);
        ctx->ItemClick(ENUM_SEL(FTB1_THRU));
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].type_bal, FTB1_THRU);
    };

    ImGuiTest* filter1__cutoff = IM_REGISTER_TEST(e, "filter1", "cutoff");
    filter1__cutoff->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_filter1_sel);
        ctx->SetRef(filt_cutoff_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].cutoff, 0);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].cutoff, 127);
    };

    ImGuiTest* filter1__resonance = IM_REGISTER_TEST(e, "filter1", "resonance");
    filter1__resonance->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_filter1_sel);
        ctx->SetRef(filt_resonance_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].resonance, 0);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].resonance, 127);
    };

    ImGuiTest* filter1__eg1_int = IM_REGISTER_TEST(e, "filter1", "eg1_int");
    filter1__eg1_int->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_filter1_sel);
        ctx->SetRef(filt_eg1_int_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].eg1_int, -63);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].eg1_int, 63);
    };

    ImGuiTest* filter1__key_trk = IM_REGISTER_TEST(e, "filter1", "key_trk");
    filter1__key_trk->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_filter1_sel);
        ctx->SetRef(filt_key_trk_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].key_trk, -63);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].key_trk, 63);
    };

    ImGuiTest* filter1__vel_sens = IM_REGISTER_TEST(e, "filter1", "vel_sens");
    filter1__vel_sens->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_filter1_sel);
        ctx->SetRef(filt_vel_sens_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].vel_sens, -63);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[0].vel_sens, 63);
    };

    ImGuiTest* filter2__balance_type = IM_REGISTER_TEST(e, "filter2", "balance_type");
    filter2__balance_type->TestFunc = [&](ImGuiTestContext* ctx) {
        enum_click(ctx, win_filter2_sel, filt_type_bal_sel);
        ctx->ItemClick(ENUM_SEL(FTB2_12LPF));
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].type_bal, FTB2_12LPF);

        enum_click(ctx, win_filter2_sel, filt_type_bal_sel);
        ctx->ItemClick(ENUM_SEL(FTB2_HPF));
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].type_bal, FTB2_HPF);

        enum_click(ctx, win_filter2_sel, filt_type_bal_sel);
        ctx->ItemClick(ENUM_SEL(FTB2_BPF));
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].type_bal, FTB2_BPF);
    };

    ImGuiTest* filter2__cutoff = IM_REGISTER_TEST(e, "filter2", "cutoff");
    filter2__cutoff->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_filter2_sel);
        ctx->SetRef(filt_cutoff_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].cutoff, 0);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].cutoff, 127);
    };

    ImGuiTest* filter2__resonance = IM_REGISTER_TEST(e, "filter2", "resonance");
    filter2__resonance->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_filter2_sel);
        ctx->SetRef(filt_resonance_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].resonance, 0);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].resonance, 127);
    };

    ImGuiTest* filter2__eg1_int = IM_REGISTER_TEST(e, "filter2", "eg1_int");
    filter2__eg1_int->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_filter2_sel);
        ctx->SetRef(filt_eg1_int_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].eg1_int, -63);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].eg1_int, 63);
    };

    ImGuiTest* filter2__key_trk = IM_REGISTER_TEST(e, "filter2", "key_trk");
    filter2__key_trk->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_filter2_sel);
        ctx->SetRef(filt_key_trk_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].key_trk, -63);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].key_trk, 63);
    };

    ImGuiTest* filter2__vel_sens = IM_REGISTER_TEST(e, "filter2", "vel_sens");
    filter2__vel_sens->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_filter2_sel);
        ctx->SetRef(filt_vel_sens_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].vel_sens, -63);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->filter_arr[1].vel_sens, 63);
    };

    ImGuiTest* eg1__vel_sens = IM_REGISTER_TEST(e, "eg1", "vel_sens");
    eg1__vel_sens->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg1_sel);
        ctx->SetRef(eg_vel_sens_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[0].vel_sens, -63);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[0].vel_sens, 63);
    };

    ImGuiTest* eg1__attack = IM_REGISTER_TEST(e, "eg1", "attack");
    eg1__attack->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg1_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_attack_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[0].attack, 0);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[0].attack, 127);
    };

    ImGuiTest* eg1__decay = IM_REGISTER_TEST(e, "eg1", "decay");
    eg1__decay->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg1_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_decay_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[0].decay, 0);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[0].decay, 127);
    };

    ImGuiTest* eg1__sustain = IM_REGISTER_TEST(e, "eg1", "sustain");
    eg1__sustain->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg1_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_sustain_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[0].sustain, 0);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[0].sustain, 127);
    };

    ImGuiTest* eg1__release = IM_REGISTER_TEST(e, "eg1", "release");
    eg1__release->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg1_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_release_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[0].release, 0);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[0].release, 127);
    };

    ImGuiTest* eg2__vel_sens = IM_REGISTER_TEST(e, "eg2", "vel_sens");
    eg2__vel_sens->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg2_sel);
        ctx->SetRef(eg_vel_sens_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[1].vel_sens, -63);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[1].vel_sens, 63);
    };

    ImGuiTest* eg2__attack = IM_REGISTER_TEST(e, "eg2", "attack");
    eg2__attack->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg2_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_attack_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[1].attack, 0);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[1].attack, 127);
    };

    ImGuiTest* eg2__decay = IM_REGISTER_TEST(e, "eg2", "decay");
    eg2__decay->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg2_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_decay_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[1].decay, 0);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[1].decay, 127);
    };

    ImGuiTest* eg2__sustain = IM_REGISTER_TEST(e, "eg2", "sustain");
    eg2__sustain->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg2_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_sustain_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[1].sustain, 0);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[1].sustain, 127);
    };

    ImGuiTest* eg2__release = IM_REGISTER_TEST(e, "eg2", "release");
    eg2__release->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg2_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_release_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[1].release, 0);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[1].release, 127);
    };

    ImGuiTest* eg3__vel_sens = IM_REGISTER_TEST(e, "eg3", "vel_sens");
    eg3__vel_sens->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg3_sel);
        ctx->SetRef(eg_vel_sens_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[2].vel_sens, -63);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[2].vel_sens, 63);
    };

    ImGuiTest* eg3__attack = IM_REGISTER_TEST(e, "eg3", "attack");
    eg3__attack->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg3_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_attack_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[2].attack, 0);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[2].attack, 127);
    };

    ImGuiTest* eg3__decay = IM_REGISTER_TEST(e, "eg3", "decay");
    eg3__decay->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg3_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_decay_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[2].decay, 0);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[2].decay, 127);
    };

    ImGuiTest* eg3__sustain = IM_REGISTER_TEST(e, "eg3", "sustain");
    eg3__sustain->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg3_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_sustain_sel);

        ctx->ItemDragWithDelta("", {0, 130});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[2].sustain, 0);

        ctx->ItemDragWithDelta("", {0, -130});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[2].sustain, 127);
    };

    ImGuiTest* eg3__release = IM_REGISTER_TEST(e, "eg3", "release");
    eg3__release->TestFunc = [&](ImGuiTestContext* ctx) {
        ctx->SetRef(win_eg3_sel);
        ctx->SetRef(ctx->WindowInfo(eg_graph_sel).Window);
        ctx->SetRef(eg_release_sel);

        ctx->ItemDragWithDelta("", {-130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[2].release, 0);

        ctx->ItemDragWithDelta("", {130, 0});
        IM_CHECK_EQ(g_app()->selected_timbre->eg_arr[2].release, 127);
    };


    ImGuiTest* osc1__wave = IM_REGISTER_TEST(e, "osc1", "wave");
    osc1__wave->TestFunc = [&](ImGuiTestContext* ctx) {
        enum_click(ctx, win_osc1_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_WT_SAW));

        enum_click(ctx, win_osc1_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_WT_PULSE));

        enum_click(ctx, win_osc1_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_WT_SIN));

        enum_click(ctx, win_osc1_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_WT_TRIANGLE));

        enum_click(ctx, win_osc1_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_WT_FORMANT));

        enum_click(ctx, win_osc1_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_WT_NOISE));

        enum_click(ctx, win_osc1_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_WT_PCM_DWGS));

        enum_click(ctx, win_osc1_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_WT_AUDIO_IN));
    };

    ImGuiTest* osc1__mod = IM_REGISTER_TEST(e, "osc1", "mod");
    osc1__mod->TestFunc = [&](ImGuiTestContext* ctx) {
        enum_click(ctx, win_osc1_sel, osc_mod_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_OM_WAVE));

        enum_click(ctx, win_osc1_sel, osc_mod_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_OM_CROSS));

        enum_click(ctx, win_osc1_sel, osc_mod_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_OM_UNISON));

        enum_click(ctx, win_osc1_sel, osc_mod_sel);
        ctx->ItemClick(ENUM_SEL(OSC1_OM_VPM));
    };

    ImGuiTest* osc2__wave = IM_REGISTER_TEST(e, "osc2", "wave");
    osc2__wave->TestFunc = [&](ImGuiTestContext* ctx) {
        enum_click(ctx, win_osc2_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC2_WT_SAW));

        enum_click(ctx, win_osc2_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC2_WT_PULSE));

        enum_click(ctx, win_osc2_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC2_WT_SIN));

        enum_click(ctx, win_osc2_sel, osc_wave_sel);
        ctx->ItemClick(ENUM_SEL(OSC2_WT_TRIANGLE));
    };

    ImGuiTest* osc2__mod = IM_REGISTER_TEST(e, "osc2", "mod");
    osc2__mod->TestFunc = [&](ImGuiTestContext* ctx) {
        enum_click(ctx, win_osc2_sel, osc_mod_sel);
        ctx->ItemClick(ENUM_SEL(OSC2_OM_OFF));

        enum_click(ctx, win_osc2_sel, osc_mod_sel);
        ctx->ItemClick(ENUM_SEL(OSC2_OM_RING));

        enum_click(ctx, win_osc2_sel, osc_mod_sel);
        ctx->ItemClick(ENUM_SEL(OSC2_OM_SYNC));

        enum_click(ctx, win_osc2_sel, osc_mod_sel);
        ctx->ItemClick(ENUM_SEL(OSC2_OM_RINGSYNC));
    };
}

