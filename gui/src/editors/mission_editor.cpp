#include "mission_editor.h"
#include "../app.h"
#include "imgui.h"
#include <string>

// .M/.MM/.MT mission files are all plain ASCII text — display and edit directly.
static std::string s_text;
static int s_lastEntry = -2;

void DrawMissionEditor(App& app) {
    auto& ed = app.editor;

    if (ed.entryIdx != s_lastEntry) {
        s_lastEntry = ed.entryIdx;
        s_text.assign((const char*)ed.data.data(), ed.data.size());
    }

    ImGui::TextDisabled("Edit mission text below. Changes are committed on Save.");
    ImGui::Separator();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    avail.y -= 36;
    if (ImGui::InputTextMultiline("##mission", s_text.data(),
                                  s_text.size() + 1,
                                  avail,
                                  ImGuiInputTextFlags_AllowTabInput |
                                  ImGuiInputTextFlags_CallbackResize,
                                  [](ImGuiInputTextCallbackData* d) -> int {
                                      if (d->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                                          auto* s = (std::string*)d->UserData;
                                          s->resize((size_t)d->BufSize - 1);
                                          d->Buf = s->data();
                                      }
                                      return 0;
                                  }, &s_text)) {
        ed.modified = true;
    }

    if (ImGui::Button("Save")) {
        ed.data.assign(s_text.begin(), s_text.end());
        ed.modified = true;
        app.CommitEntry(ed.data);
    }
}
