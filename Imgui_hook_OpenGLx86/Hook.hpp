#include <Windows.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include <gl/GL.h>
#include "detours.h"


//-----------------------------------------------
//                 全局变量
//-----------------------------------------------
static bool g_ShowMenu = true;          // 控制菜单显示
static HWND g_hWnd = nullptr;           // 游戏窗口句柄
static WNDPROC g_OriginalWndProc = nullptr; // 原始窗口过程
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



// 原始函数声明
using wglSwapBuffers_t = BOOL(WINAPI*)(HDC);
wglSwapBuffers_t OriginalwglSwapBuffers = nullptr;

//-----------------------------------------------
//               自定义窗口过程
//-----------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // 将输入事件传递给 ImGui
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
        return true; // ImGui 已处理该事件
    }

    // 传递事件给游戏原始窗口过程
    return CallWindowProc(g_OriginalWndProc, hWnd, msg, wParam, lParam);
}

//-----------------------------------------------
//               ImGui 初始化
//-----------------------------------------------
void InitImGui(HDC hdc) {
    // 获取窗口句柄
    g_hWnd = WindowFromDC(hdc);

    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 启用键盘控制

    // 设置风格
    ImGui::StyleColorsDark();

    // 初始化 Win32 和 OpenGL 后端
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplOpenGL3_Init();

    // 备份原始窗口过程，并替换为自定义过程
    g_OriginalWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
}

//-----------------------------------------------
//              Hook 后的 wglSwapBuffers
//-----------------------------------------------
BOOL WINAPI HookedwglSwapBuffers(HDC hdc) {
    // 首次调用时初始化 ImGui
    static bool isInitialized = false;
    if (!isInitialized) {
        InitImGui(hdc);
        isInitialized = true;
    }

    // 开始新帧

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // 绘制菜单
    if (g_ShowMenu) {
        ImGui::Begin("game", &g_ShowMenu);
        ImGui::Text("hello");
        if (ImGui::Button("EXIT")) {
            g_ShowMenu = false;
        }
        ImGui::End();
    }

    // 渲染 ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 调用原始函数完成缓冲区交换
    return OriginalwglSwapBuffers(hdc);
}

//-----------------------------------------------
//               Hook 初始化与卸载
//-----------------------------------------------
void InitHook() {
    // 获取原始 wglSwapBuffers 地址
    HMODULE hOpenGL = GetModuleHandleA("opengl32.dll");
    OriginalwglSwapBuffers = (wglSwapBuffers_t)GetProcAddress(hOpenGL, "wglSwapBuffers");

    // 使用 Detours 进行 Hook
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)OriginalwglSwapBuffers, HookedwglSwapBuffers);
    DetourTransactionCommit();
}

void Unhook() {
    // 恢复原始函数
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)OriginalwglSwapBuffers, HookedwglSwapBuffers);
    DetourTransactionCommit();

    // 清理 ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // 恢复窗口过程
    SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)g_OriginalWndProc);
}