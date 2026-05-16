#include "app.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <dxgi.h>
#include <tchar.h>
#include <string>

// Forward declaration of ImGui Win32 message handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

// ---------- DX11 globals ----------
static ID3D11Device*            g_device      = nullptr;
static ID3D11DeviceContext*     g_context     = nullptr;
static IDXGISwapChain*          g_swapChain   = nullptr;
static ID3D11RenderTargetView*  g_mainRTV     = nullptr;

static bool CreateDeviceD3D(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount                        = 2;
    sd.BufferDesc.Width                   = 0;
    sd.BufferDesc.Height                  = 0;
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                       = hwnd;
    sd.SampleDesc.Count                   = 1;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
    sd.Windowed                           = TRUE;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        featureLevels, 2, D3D11_SDK_VERSION,
        &sd, &g_swapChain, &g_device, nullptr, &g_context);
    if (FAILED(hr)) return false;

    ID3D11Texture2D* backBuffer = nullptr;
    g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    g_device->CreateRenderTargetView(backBuffer, nullptr, &g_mainRTV);
    backBuffer->Release();
    return true;
}

static void CleanupDeviceD3D() {
    if (g_mainRTV) { g_mainRTV->Release(); g_mainRTV = nullptr; }
    if (g_swapChain) { g_swapChain->Release(); g_swapChain = nullptr; }
    if (g_context)  { g_context->Release();  g_context  = nullptr; }
    if (g_device)   { g_device->Release();   g_device   = nullptr; }
}

static void CreateRenderTarget() {
    ID3D11Texture2D* backBuffer = nullptr;
    g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    g_device->CreateRenderTargetView(backBuffer, nullptr, &g_mainRTV);
    backBuffer->Release();
}

static void CleanupRenderTarget() {
    if (g_mainRTV) { g_mainRTV->Release(); g_mainRTV = nullptr; }
}

// ---------- Win32 window proc ----------
static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp)) return true;
    switch (msg) {
    case WM_SIZE:
        if (g_device && wp != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_swapChain->ResizeBuffers(0, LOWORD(lp), HIWORD(lp),
                                       DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wp & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ---------- WinMain ----------
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_CLASSDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"FT_GUI";
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowW(L"FT_GUI", L"Fighters Toolkit",
        WS_OVERLAPPEDWINDOW, 100, 100, 1400, 900,
        nullptr, nullptr, hInst, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, hInst);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "ft-gui.ini";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    // Slightly soften the default dark theme
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding  = 4.0f;
    style.FrameRounding   = 3.0f;
    style.GrabRounding    = 3.0f;
    style.ScrollbarRounding = 3.0f;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_device, g_context);

    App app(g_device, g_context);

    bool done = false;
    while (!done) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        app.Draw();

        ImGui::Render();
        const float clearColor[4] = { 0.12f, 0.12f, 0.12f, 1.0f };
        g_context->OMSetRenderTargets(1, &g_mainRTV, nullptr);
        g_context->ClearRenderTargetView(g_mainRTV, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_swapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, hInst);
    return 0;
}
