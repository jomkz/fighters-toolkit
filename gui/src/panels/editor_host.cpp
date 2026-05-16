#include "editor_host.h"
#include "../app.h"
#include "../editors/brf_editor.h"
#include "../editors/pic_editor.h"
#include "../editors/audio_editor.h"
#include "../editors/mission_editor.h"
#include "../editors/seq_editor.h"
#include "../editors/inf_editor.h"
#include "../editors/plt_editor.h"
#include "../editors/raw_viewer.h"
#include "imgui.h"

void DrawEditorHost(App& app) {
    if (app.editor.kind == EditorKind::None) {
        ImGui::TextDisabled("Select a record in the LIB Browser to edit.");
        return;
    }

    // Record title bar
    if (app.editor.libIdx >= 0 && app.editor.libIdx < (int)app.sessions.size()) {
        const auto& e = app.sessions[app.editor.libIdx]
                            .entries[app.editor.entryIdx];
        ImGui::Text("%s", e.name);
        if (app.editor.modified) {
            ImGui::SameLine();
            ImGui::TextColored({1,0.8f,0,1}, "(modified)");
            ImGui::SameLine();
            if (ImGui::SmallButton("Commit")) {
                // Editors write back to editor.data when saving;
                // CommitEntry patches it into the session.
                app.CommitEntry(app.editor.data);
            }
        }
        ImGui::Separator();
    }

    switch (app.editor.kind) {
    case EditorKind::Brf:     DrawBrfEditor(app);     break;
    case EditorKind::Pic:     DrawPicEditor(app);     break;
    case EditorKind::Audio:   DrawAudioEditor(app);   break;
    case EditorKind::Mission: DrawMissionEditor(app); break;
    case EditorKind::Seq:     DrawSeqEditor(app);     break;
    case EditorKind::Inf:     DrawInfEditor(app);     break;
    case EditorKind::Plt:     DrawPltEditor(app);     break;
    case EditorKind::Raw:     DrawRawViewer(app);     break;
    default:
        ImGui::TextDisabled("No editor for .%s files.", app.editor.ext.c_str());
        break;
    }

}
