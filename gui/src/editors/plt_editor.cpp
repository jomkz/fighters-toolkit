#include "plt_editor.h"
#include "../app.h"
#include "imgui.h"
#include <cstring>
#include <cstdio>

// PLT identity block offsets (from docs/formats/PLT.md)
static const int OFF_NAME     = 0x01;  // 63 bytes
static const int OFF_CALLSIGN = 0x40;  // 32 bytes
static const int OFF_VOICE    = 0x61;  // 13 bytes
static const int OFF_NOSE     = 0x6E;  // 13 bytes
static const int OFF_LEFT     = 0x7B;  // 13 bytes
static const int OFF_RIGHT    = 0x88;  // 13 bytes
static const int OFF_PORTRAIT = 0x95;  // 13 bytes
static const int OFF_RANK     = 0xA2;  // 14 bytes
static const int PLT_MIN_SIZE = 0xB0;

static char s_name[64]     = {};
static char s_callsign[33] = {};
static char s_voice[14]    = {};
static char s_nose[14]     = {};
static char s_left[14]     = {};
static char s_right[14]    = {};
static char s_portrait[14] = {};
static char s_rank[15]     = {};
static int  s_lastEntry    = -2;

static void ReadField(const std::vector<uint8_t>& data, int off, char* buf, int len) {
    if (off + len > (int)data.size()) { buf[0] = 0; return; }
    memcpy(buf, data.data() + off, (size_t)(len - 1));
    buf[len - 1] = 0;
}
static void WriteField(std::vector<uint8_t>& data, int off, const char* buf, int len) {
    if (off + len > (int)data.size()) return;
    memset(data.data() + off, 0, (size_t)len);
    strncpy_s((char*)data.data() + off, (size_t)len, buf, (size_t)(len - 1));
}

void DrawPltEditor(App& app) {
    auto& ed = app.editor;

    if ((int)ed.data.size() < PLT_MIN_SIZE) {
        ImGui::TextColored({1,0.4f,0.4f,1},
            "File too small to be a valid PLT (%zu bytes).", ed.data.size());
        return;
    }

    if (ed.entryIdx != s_lastEntry) {
        s_lastEntry = ed.entryIdx;
        ReadField(ed.data, OFF_NAME,     s_name,     sizeof(s_name));
        ReadField(ed.data, OFF_CALLSIGN, s_callsign, sizeof(s_callsign));
        ReadField(ed.data, OFF_VOICE,    s_voice,    sizeof(s_voice));
        ReadField(ed.data, OFF_NOSE,     s_nose,     sizeof(s_nose));
        ReadField(ed.data, OFF_LEFT,     s_left,     sizeof(s_left));
        ReadField(ed.data, OFF_RIGHT,    s_right,    sizeof(s_right));
        ReadField(ed.data, OFF_PORTRAIT, s_portrait, sizeof(s_portrait));
        ReadField(ed.data, OFF_RANK,     s_rank,     sizeof(s_rank));
    }

    ImGui::SeparatorText("Identity");

    bool changed = false;
    auto field = [&](const char* label, char* buf, int bufSize) {
        ImGui::SetNextItemWidth(220);
        if (ImGui::InputText(label, buf, (size_t)bufSize))
            changed = true;
    };

    field("Pilot name",    s_name,     sizeof(s_name));
    field("Callsign",      s_callsign, sizeof(s_callsign));
    field("Voice file",    s_voice,    sizeof(s_voice));
    field("Rank",          s_rank,     sizeof(s_rank));

    ImGui::SeparatorText("Cosmetics");
    field("Portrait ID",   s_portrait, sizeof(s_portrait));
    field("Nose art ID",   s_nose,     sizeof(s_nose));
    field("Left decal ID", s_left,     sizeof(s_left));
    field("Right decal ID",s_right,    sizeof(s_right));

    ImGui::Separator();
    ImGui::TextDisabled("Stats block (0xB0-0x0D7E): pending second differential pass.");

    if (changed) {
        WriteField(ed.data, OFF_NAME,     s_name,     sizeof(s_name));
        WriteField(ed.data, OFF_CALLSIGN, s_callsign, sizeof(s_callsign));
        WriteField(ed.data, OFF_VOICE,    s_voice,    sizeof(s_voice));
        WriteField(ed.data, OFF_NOSE,     s_nose,     sizeof(s_nose));
        WriteField(ed.data, OFF_LEFT,     s_left,     sizeof(s_left));
        WriteField(ed.data, OFF_RIGHT,    s_right,    sizeof(s_right));
        WriteField(ed.data, OFF_PORTRAIT, s_portrait, sizeof(s_portrait));
        WriteField(ed.data, OFF_RANK,     s_rank,     sizeof(s_rank));
        ed.modified = true;
    }

    if (ed.modified && ImGui::Button("Save")) {
        app.CommitEntry(ed.data);
    }
}
