#pragma once
#include "imgui.h"
#include "ft/ealib.h"
#include <d3d11.h>
#include <string>
#include <vector>
#include <functional>

// A single open LIB file.
struct LibSession {
    std::string              path;       // full path to the .LIB file
    std::vector<uint8_t>     data;       // raw file bytes
    std::vector<ft::Entry>   entries;    // parsed directory
    bool                     dirty = false;
};

// What is currently open in the editor center panel.
enum class EditorKind {
    None,
    Brf,        // OT/NT/PT/JT/SEE/ECM/GAS
    Pic,
    Audio,
    Mission,
    Seq,
    Inf,
    Plt,
    Raw,
};

struct EditorState {
    EditorKind   kind     = EditorKind::None;
    int          libIdx   = -1;    // index into App::sessions
    int          entryIdx = -1;    // index into LibSession::entries
    std::string  ext;              // lowercase extension
    std::vector<uint8_t> data;     // decompressed record bytes
    bool         modified = false;
};

// Texture handle for image preview.
struct GpuTexture {
    ID3D11ShaderResourceView* srv = nullptr;
    int width  = 0;
    int height = 0;
    void Release() { if (srv) { srv->Release(); srv = nullptr; } }
};

class App {
public:
    App(ID3D11Device* device, ID3D11DeviceContext* ctx);
    ~App();
    void Draw();

    // Called by panels/editors to open a record for editing.
    void OpenEntry(int libIdx, int entryIdx);

    // Save modified record back into the session (patches in memory).
    void CommitEntry(const std::vector<uint8_t>& newData);

    // Write the session's in-memory LIB to FA_0.LIB in the configured install dir.
    void InstallToGame(int libIdx);

    // Upload RGBA pixels to a DX11 texture for display in ImGui.
    GpuTexture UploadTexture(const uint8_t* rgba, int w, int h);

    // ---------- public state ----------
    std::vector<LibSession> sessions;
    EditorState             editor;
    std::string             installDir;   // FA game directory
    std::string             statusMsg;

private:
    void DrawMenuBar();

    void OpenLibDialog();
    void SaveSessionDialog(int libIdx);
    void ChooseInstallDir();

    ID3D11Device*        m_device;
    ID3D11DeviceContext* m_ctx;
};
