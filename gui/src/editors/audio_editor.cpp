#include "audio_editor.h"
#include "../app.h"
#include "imgui.h"
#include "ft/audio.h"
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <commdlg.h>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

static std::string Win32SaveFile(const wchar_t* filter, const wchar_t* defExt) {
    wchar_t buf[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = (HWND)ImGui::GetMainViewport()->PlatformHandleRaw;
    ofn.lpstrFilter  = filter;
    ofn.lpstrFile    = buf;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrDefExt  = defExt;
    ofn.Flags        = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    if (!GetSaveFileNameW(&ofn)) return {};
    int len = WideCharToMultiByte(CP_UTF8,0,buf,-1,nullptr,0,nullptr,nullptr);
    std::string s(len-1,0); WideCharToMultiByte(CP_UTF8,0,buf,-1,s.data(),len,nullptr,nullptr);
    return s;
}
static std::string Win32OpenFile(const wchar_t* filter, const wchar_t* title) {
    wchar_t buf[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = (HWND)ImGui::GetMainViewport()->PlatformHandleRaw;
    ofn.lpstrFilter  = filter;
    ofn.lpstrFile    = buf;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrTitle   = title;
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn)) return {};
    int len = WideCharToMultiByte(CP_UTF8,0,buf,-1,nullptr,0,nullptr,nullptr);
    std::string s(len-1,0); WideCharToMultiByte(CP_UTF8,0,buf,-1,s.data(),len,nullptr,nullptr);
    return s;
}

// Infer sample rate from extension stored in editor.ext
static int InferRate(const std::string& ext) {
    if (ext == "5k")  return 5000;
    if (ext == "8k")  return 8000;
    if (ext == "22k") return 22050;
    return 11025;
}

void DrawAudioEditor(App& app) {
    auto& ed = app.editor;
    int rate = InferRate(ed.ext);
    int samples = (int)ed.data.size();
    float duration = samples / (float)rate;

    ImGui::Text("Sample rate: %d Hz  |  Samples: %d  |  Duration: %.2f s",
                rate, samples, duration);
    ImGui::Separator();

    // Simple waveform display (downsampled to ~512 points)
    const int DISP = 512;
    static float waveform[DISP];
    static int   lastEntry = -2;
    if (lastEntry != ed.entryIdx) {
        lastEntry = ed.entryIdx;
        for (int i = 0; i < DISP; i++) {
            int idx = (int)((float)i / DISP * samples);
            if (idx < samples)
                waveform[i] = ((float)ed.data[idx] - 128.0f) / 128.0f;
            else
                waveform[i] = 0.0f;
        }
    }
    ImGui::PlotLines("##wave", waveform, DISP, 0, nullptr, -1.0f, 1.0f,
                     ImVec2(-1, 80));

    ImGui::Separator();

    if (ImGui::Button("Play")) {
        // Build a minimal WAV in memory and play via waveOut
        std::vector<uint8_t> wav;
        wav.resize(44 + samples);
        auto w16 = [&](int off, uint16_t v) { memcpy(&wav[off], &v, 2); };
        auto w32 = [&](int off, uint32_t v) { memcpy(&wav[off], &v, 4); };
        memcpy(&wav[0], "RIFF", 4); w32(4, (uint32_t)(36 + samples));
        memcpy(&wav[8], "WAVEfmt ", 8); w32(16, 16);
        w16(20, 1); w16(22, 1); w32(24, (uint32_t)rate); w32(28, (uint32_t)rate);
        w16(32, 1); w16(34, 8);
        memcpy(&wav[36], "data", 4); w32(40, (uint32_t)samples);
        memcpy(&wav[44], ed.data.data(), (size_t)samples);
        PlaySoundA((LPCSTR)wav.data(), nullptr,
                   SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
        PlaySoundA(nullptr, nullptr, 0);

    ImGui::Separator();

    if (ImGui::Button("Export WAV...")) {
        std::string path = Win32SaveFile(L"WAV Audio\0*.wav\0All Files\0*.*\0", L"wav");
        if (!path.empty()) {
            auto wav = ft::audio_to_wav(ed.data.data(), ed.data.size(), (uint32_t)rate);
            if (!wav.empty()) {
                std::ofstream f(path, std::ios::binary);
                if (f) f.write((const char*)wav.data(), (std::streamsize)wav.size());
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Import WAV...")) {
        std::string path = Win32OpenFile(L"WAV Audio\0*.wav\0All Files\0*.*\0",
                                         L"Import WAV");
        if (!path.empty()) {
            std::ifstream f(path, std::ios::binary | std::ios::ate);
            if (f) {
                auto sz = f.tellg(); f.seekg(0);
                std::vector<uint8_t> wav((size_t)sz);
                f.read((char*)wav.data(), (std::streamsize)sz);
                uint32_t outRate = 0;
                auto pcm = ft::wav_to_pcm(wav.data(), wav.size(), &outRate);
                if (!pcm.empty()) {
                    ed.data     = std::move(pcm);
                    ed.modified = true;
                    app.statusMsg = "Imported " + fs::path(path).filename().string();
                }
            }
        }
    }
}
