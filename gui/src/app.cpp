#include "app.h"
#include "panels/lib_browser.h"
#include "panels/editor_host.h"
#include "panels/preview.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include <fstream>
#include <shlobj.h>
#include <commdlg.h>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

App::App(ID3D11Device* device, ID3D11DeviceContext* ctx)
    : m_device(device), m_ctx(ctx) {}

App::~App() {}

void App::Draw() {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    float menuH = ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.y * 2.0f;
    float y0    = vp->WorkPos.y + menuH;
    float h     = vp->WorkSize.y - menuH;

    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, y0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(290, h), ImGuiCond_FirstUseEver);
    DrawLibBrowser(*this);

    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + 295, y0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(820, h), ImGuiCond_FirstUseEver);
    DrawEditorHost(*this);

    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + 1120, y0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(275, h), ImGuiCond_FirstUseEver);
    DrawPreview(*this);

    DrawMenuBar();
}

// ---------- Menu bar ----------

void App::DrawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open LIB...", "Ctrl+O"))  OpenLibDialog();
            ImGui::Separator();
            if (ImGui::MenuItem("Choose FA Install Dir...")) ChooseInstallDir();
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4"))
                PostQuitMessage(0);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            for (int i = 0; i < (int)sessions.size(); i++) {
                std::string label = "Install " +
                    fs::path(sessions[i].path).filename().string() +
                    " as FA_0.LIB";
                if (ImGui::MenuItem(label.c_str(), nullptr, false,
                                    !installDir.empty()))
                    InstallToGame(i);
            }
            if (sessions.empty())
                ImGui::TextDisabled("(no LIB open)");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About"))
                ImGui::OpenPopup("##About");
            ImGui::EndMenu();
        }

        // Status message on the right
        if (!statusMsg.empty()) {
            float w = ImGui::CalcTextSize(statusMsg.c_str()).x + 16.0f;
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - w);
            ImGui::TextDisabled("%s", statusMsg.c_str());
        }
        ImGui::EndMainMenuBar();
    }

    // About popup
    if (ImGui::BeginPopupModal("##About", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Fighters Toolkit GUI");
        ImGui::Text("Modern replacement for FATK (DuoSoft 1998)");
        ImGui::Separator();
        ImGui::Text("Backend: ft_lib (C++17)");
        ImGui::Text("GUI: Dear ImGui + DirectX 11");
        if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

// ---------- File dialogs ----------

static std::string Win32OpenFile(const wchar_t* filter, const wchar_t* title) {
    wchar_t buf[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filter;
    ofn.lpstrFile   = buf;
    ofn.nMaxFile    = MAX_PATH;
    ofn.lpstrTitle  = title;
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn)) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
    std::string s(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, buf, -1, s.data(), len, nullptr, nullptr);
    return s;
}

void App::OpenLibDialog() {
    std::string path = Win32OpenFile(
        L"LIB Files\0*.LIB\0All Files\0*.*\0", L"Open LIB File");
    if (path.empty()) return;

    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) { statusMsg = "Cannot open: " + path; return; }
    auto sz = f.tellg(); f.seekg(0);
    LibSession s;
    s.path = path;
    s.data.resize((size_t)sz);
    f.read((char*)s.data.data(), sz);
    s.entries = ft::ealib_read_dir(s.data.data(), s.data.size());
    if (s.entries.empty()) { statusMsg = "Not a valid LIB: " + path; return; }
    sessions.push_back(std::move(s));
    statusMsg = "Opened " + fs::path(path).filename().string() +
                " (" + std::to_string(sessions.back().entries.size()) + " entries)";
}

void App::ChooseInstallDir() {
    wchar_t buf[MAX_PATH] = {};
    BROWSEINFOW bi = {};
    bi.lpszTitle = L"Select FA install directory";
    bi.ulFlags   = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (!pidl) return;
    SHGetPathFromIDListW(pidl, buf);
    CoTaskMemFree(pidl);
    int len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
    installDir.assign(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, buf, -1, installDir.data(), len, nullptr, nullptr);
    statusMsg = "Install dir: " + installDir;
}

// ---------- Entry open / commit ----------

void App::OpenEntry(int libIdx, int entryIdx) {
    if (libIdx < 0 || libIdx >= (int)sessions.size()) return;
    const auto& s = sessions[libIdx];
    if (entryIdx < 0 || entryIdx >= (int)s.entries.size()) return;

    EditorState es;
    es.libIdx   = libIdx;
    es.entryIdx = entryIdx;
    es.data     = ft::ealib_extract(s.data.data(), s.data.size(),
                                    s.entries[entryIdx], true);

    std::string name = s.entries[entryIdx].name;
    auto dot = name.rfind('.');
    es.ext = (dot != std::string::npos) ? name.substr(dot + 1) : "";
    for (auto& c : es.ext) c = (char)tolower(c);

    const char* brfExts[] = { "ot","nt","pt","jt","see","ecm","gas" };
    for (auto* x : brfExts)
        if (es.ext == x) { es.kind = EditorKind::Brf; break; }
    if      (es.ext == "pic")                      es.kind = EditorKind::Pic;
    else if (es.ext == "11k" || es.ext == "5k" ||
             es.ext == "8k"  || es.ext == "22k")   es.kind = EditorKind::Audio;
    else if (es.ext == "m"   || es.ext == "mm" ||
             es.ext == "mt")                       es.kind = EditorKind::Mission;
    else if (es.ext == "seq")                      es.kind = EditorKind::Seq;
    else if (es.ext == "inf")                      es.kind = EditorKind::Inf;
    else if (es.ext == "raw")                      es.kind = EditorKind::Raw;

    editor = std::move(es);
}

void App::CommitEntry(const std::vector<uint8_t>& newData) {
    if (editor.libIdx < 0) return;
    auto& s = sessions[editor.libIdx];
    std::string name = s.entries[editor.entryIdx].name;
    s.data  = ft::ealib_patch(s.data.data(), s.data.size(), name, newData);
    s.entries = ft::ealib_read_dir(s.data.data(), s.data.size());
    s.dirty = true;
    editor.modified = false;
    statusMsg = std::string("Patched ") + name;
}

void App::InstallToGame(int libIdx) {
    if (libIdx < 0 || libIdx >= (int)sessions.size()) return;
    if (installDir.empty()) { statusMsg = "Set FA install dir first."; return; }
    std::string dest = installDir + "\\FA_0.LIB";
    std::ofstream f(dest, std::ios::binary);
    if (!f) { statusMsg = "Cannot write: " + dest; return; }
    const auto& data = sessions[libIdx].data;
    f.write((const char*)data.data(), (std::streamsize)data.size());
    statusMsg = "Installed to " + dest;
}

// ---------- GPU texture upload ----------

GpuTexture App::UploadTexture(const uint8_t* rgba, int w, int h) {
    GpuTexture t;
    if (!rgba || w <= 0 || h <= 0) return t;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width              = (UINT)w;
    desc.Height             = (UINT)h;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count   = 1;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA init = {};
    init.pSysMem     = rgba;
    init.SysMemPitch = (UINT)(w * 4);

    ID3D11Texture2D* tex = nullptr;
    if (FAILED(m_device->CreateTexture2D(&desc, &init, &tex))) return t;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels       = 1;
    if (FAILED(m_device->CreateShaderResourceView(tex, &srvDesc, &t.srv))) {
        tex->Release(); return t;
    }
    tex->Release();
    t.width  = w;
    t.height = h;
    return t;
}
