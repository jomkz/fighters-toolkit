#include "lib_browser.h"
#include "../app.h"
#include "imgui.h"
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

static bool ci_contains(const char* hay, const char* needle) {
    if (!needle || needle[0] == '\0') return true;
    size_t nl = strlen(needle);
    for (const char* h = hay; *h; h++)
        if (_strnicmp(h, needle, nl) == 0) return true;
    return false;
}

// Extension → human-readable type label
static const char* EntryTypeLabel(const char* name) {
    const char* dot = strrchr(name, '.');
    if (!dot) return "DAT";
    const char* e = dot + 1;
    if (_stricmp(e,"PT")==0)  return "Aircraft";
    if (_stricmp(e,"OT")==0)  return "Object";
    if (_stricmp(e,"NT")==0)  return "NPC";
    if (_stricmp(e,"JT")==0)  return "Ordnance";
    if (_stricmp(e,"SEE")==0) return "Seeker";
    if (_stricmp(e,"ECM")==0) return "ECM";
    if (_stricmp(e,"GAS")==0) return "Gas";
    if (_stricmp(e,"PIC")==0) return "Image";
    if (_stricmp(e,"PAL")==0) return "Palette";
    if (_stricmp(e,"SEQ")==0) return "Sequence";
    if (_stricmp(e,"11K")==0||_stricmp(e,"5K")==0||
        _stricmp(e,"8K")==0||_stricmp(e,"22K")==0) return "Audio";
    if (_stricmp(e,"M")==0)   return "Mission";
    if (_stricmp(e,"MM")==0)  return "Map";
    if (_stricmp(e,"MT")==0)  return "Briefing";
    if (_stricmp(e,"SH")==0)  return "Shape";
    if (_stricmp(e,"CB8")==0) return "FMV";
    if (_stricmp(e,"VDO")==0) return "Video";
    if (_stricmp(e,"FBC")==0) return "FrameIdx";
    if (_stricmp(e,"INF")==0) return "TechInfo";
    if (_stricmp(e,"HUD")==0) return "HUD";
    if (_stricmp(e,"MUS")==0) return "Music";
    return "File";
}

static ImVec4 TypeColor(const char* label) {
    if (strcmp(label,"Aircraft")==0) return {0.4f,0.8f,1.0f,1};
    if (strcmp(label,"Ordnance")==0) return {1.0f,0.6f,0.3f,1};
    if (strcmp(label,"Image")==0)    return {0.6f,1.0f,0.6f,1};
    if (strcmp(label,"Audio")==0)    return {1.0f,0.9f,0.4f,1};
    if (strcmp(label,"Mission")==0||strcmp(label,"Map")==0||
        strcmp(label,"Briefing")==0) return {0.9f,0.6f,1.0f,1};
    if (strcmp(label,"Shape")==0)    return {1.0f,0.7f,0.7f,1};
    if (strcmp(label,"FMV")==0||strcmp(label,"Video")==0)
                                     return {0.7f,0.7f,0.7f,1};
    return {0.75f,0.75f,0.75f,1};
}

void DrawLibBrowser(App& app) {
    ImGui::Begin("LIB Browser");

    if (app.sessions.empty()) {
        ImGui::TextDisabled("No LIB open.");
        ImGui::TextDisabled("File -> Open LIB...");
        ImGui::End();
        return;
    }

    // Filter bar
    static char filter[64] = {};
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##filter", "Filter...", filter, sizeof(filter));
    ImGui::Separator();

    for (int si = 0; si < (int)app.sessions.size(); si++) {
        auto& sess = app.sessions[si];
        std::string libName = fs::path(sess.path).filename().string();
        if (sess.dirty) libName += " *";
        std::string nodeId = libName + "##lib" + std::to_string(si);

        bool open = ImGui::TreeNodeEx(nodeId.c_str(),
                        ImGuiTreeNodeFlags_DefaultOpen |
                        ImGuiTreeNodeFlags_SpanAvailWidth);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s\n%zu entries", sess.path.c_str(),
                              sess.entries.size());
        // Right-click context on the library node
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Close")) {
                if (open) ImGui::TreePop();
                open = false;
                app.sessions.erase(app.sessions.begin() + si);
                ImGui::EndPopup();
                ImGui::End();
                return;
            }
            if (ImGui::MenuItem("Install as FA_0.LIB",
                                nullptr, false, !app.installDir.empty()))
                app.InstallToGame(si);
            ImGui::EndPopup();
        }

        if (!open) continue;

        // Column headers
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4,2));
        if (ImGui::BeginTable("##entries", 3,
                ImGuiTableFlags_ScrollY |
                ImGuiTableFlags_RowBg   |
                ImGuiTableFlags_BordersInnerV |
                ImGuiTableFlags_Resizable,
                ImVec2(0, 0))) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Name",  ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Type",  ImGuiTableColumnFlags_WidthFixed, 72);
            ImGui::TableSetupColumn("Size",  ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableHeadersRow();

            for (int ei = 0; ei < (int)sess.entries.size(); ei++) {
                const auto& e = sess.entries[ei];
                const char* label = EntryTypeLabel(e.name);
                // Apply filter
                if (filter[0] != '\0') {
                    if (!ci_contains(e.name, filter) &&
                        !ci_contains(label, filter))
                        continue;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                bool selected = (app.editor.libIdx == si &&
                                 app.editor.entryIdx == ei);
                ImGui::PushID(ei);
                if (ImGui::Selectable(e.name, selected,
                                      ImGuiSelectableFlags_SpanAllColumns |
                                      ImGuiSelectableFlags_AllowDoubleClick)) {
                    app.OpenEntry(si, ei);
                }
                ImGui::PopID();

                ImGui::TableSetColumnIndex(1);
                ImVec4 col = TypeColor(label);
                ImGui::TextColored(col, "%s", label);

                ImGui::TableSetColumnIndex(2);
                if (e.size < 1024)
                    ImGui::Text("%u B", e.size);
                else
                    ImGui::Text("%.1f K", e.size / 1024.0f);
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::TreePop();
    }

    ImGui::End();
}
