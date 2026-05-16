#include "inf_editor.h"
#include "../app.h"
#include "imgui.h"
#include <string>

// INF files are RTF. Display as raw text in a scrollable editor.
// Full WYSIWYG RTF rendering would require an embedded RichEdit HWND (Phase 3).
static std::string s_text;
static int s_lastEntry = -2;

void DrawInfEditor(App& app) {
    auto& ed = app.editor;

    if (ed.entryIdx != s_lastEntry) {
        s_lastEntry = ed.entryIdx;
        s_text.assign((const char*)ed.data.data(), ed.data.size());
    }

    ImGui::TextDisabled("INF files are RTF. Editing raw RTF source below.");
    ImGui::TextDisabled("Full WYSIWYG editor: Phase 3 (embedded RichEdit).");
    ImGui::Separator();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    avail.y -= 36;
    if (ImGui::InputTextMultiline("##inf", s_text.data(), s_text.size() + 1,
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
        app.CommitEntry(ed.data);
    }
}
