#include "preview.h"
#include "../app.h"
#include "imgui.h"
#include "ft/pic.h"
#include "ft/pal.h"
#include "ft/raw.h"

// Cached preview texture so we only re-upload when the selection changes.
static GpuTexture s_preview;
static int        s_previewLib   = -2;
static int        s_previewEntry = -2;

void DrawPreview(App& app) {

    const auto& ed = app.editor;

    // Rebuild preview texture when selection changes
    if (ed.libIdx != s_previewLib || ed.entryIdx != s_previewEntry) {
        s_preview.Release();
        s_previewLib   = ed.libIdx;
        s_previewEntry = ed.entryIdx;

        if (!ed.data.empty()) {
            if (ed.kind == EditorKind::Pic) {
                ft::PicInfo info;
                if (ft::pic_info(ed.data.data(), ed.data.size(), &info)) {
                    auto rgba = ft::pic_decode(ed.data.data(), ed.data.size(), nullptr);
                    if (!rgba.empty())
                        s_preview = app.UploadTexture(rgba.data(),
                                                      (int)info.width, (int)info.height);
                }
            } else if (ed.kind == EditorKind::Raw) {
                ft::RawInfo info;
                if (ft::raw_info(ed.data.data(), ed.data.size(), &info)) {
                    auto rgba = ft::raw_decode(ed.data.data(), ed.data.size());
                    if (!rgba.empty())
                        s_preview = app.UploadTexture(rgba.data(),
                                                      (int)info.width, (int)info.height);
                }
            }
        }
    }

    if (s_preview.srv) {
        float avail = ImGui::GetContentRegionAvail().x;
        float scale = avail / (float)s_preview.width;
        float dispH = s_preview.height * scale;
        ImGui::Image((ImTextureID)s_preview.srv, ImVec2(avail, dispH));
        ImGui::TextDisabled("%dx%d", s_preview.width, s_preview.height);
    } else if (ed.kind != EditorKind::None) {
        ImGui::TextDisabled("No preview for .%s", ed.ext.c_str());
    } else {
        ImGui::TextDisabled("No record selected.");
    }

}
