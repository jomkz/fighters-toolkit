#include "seq_editor.h"
#include "../app.h"
#include "imgui.h"
#include "ft/seq.h"
#include <string>
#include <cstring>

static ft::SeqFile s_seq;
static int s_lastEntry = -2;

void DrawSeqEditor(App& app) {
    auto& ed = app.editor;

    if (ed.entryIdx != s_lastEntry) {
        s_lastEntry = ed.entryIdx;
        s_seq = ft::seq_parse(ed.data.data(), ed.data.size());
    }

    int evCount = 0;
    for (bool b : s_seq.is_event) if (b) evCount++;
    ImGui::TextDisabled("%d events", evCount);
    ImGui::Separator();

    if (ImGui::BeginTable("##seq", 2,
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Cmd",  ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        int rowId = 0;
        for (size_t i = 0; i < s_seq.lines.size(); ++i) {
            if (!s_seq.is_event[i]) continue;
            auto& ev = s_seq.events[i];
            ImGui::TableNextRow();
            ImGui::PushID(rowId++);

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ev.command.c_str());

            ImGui::TableSetColumnIndex(1);
            char rawBuf[512];
            size_t copyLen = ev.raw.size() < sizeof(rawBuf) - 1
                             ? ev.raw.size() : sizeof(rawBuf) - 1;
            memcpy(rawBuf, ev.raw.c_str(), copyLen);
            rawBuf[copyLen] = '\0';
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##r", rawBuf, sizeof(rawBuf))) {
                ev.raw = rawBuf;
                s_seq.lines[i] = "\t" + ev.raw;
                ed.modified = true;
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (ed.modified) {
        ed.data = ft::seq_serialize(s_seq);
    }
}
