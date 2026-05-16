#include "raw_viewer.h"
#include "../app.h"
#include "imgui.h"
#include "ft/raw.h"

void DrawRawViewer(App& app) {
    auto& ed = app.editor;
    ft::RawInfo info;
    if (!ft::raw_info(ed.data.data(), ed.data.size(), &info)) {
        ImGui::TextColored({1,0.4f,0.4f,1}, "Not a valid RAW screenshot.");
        return;
    }
    ImGui::Text("Resolution: %u x %u", info.width, info.height);
    ImGui::Text("File size:  %zu bytes", ed.data.size());
    ImGui::Separator();
    ImGui::TextDisabled("Image shown in the Preview panel.");
    ImGui::TextDisabled("Use  ft raw unpack  to export as PNG.");
}
