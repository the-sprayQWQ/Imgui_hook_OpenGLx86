#include <Windows.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include <gl/GL.h>
#include "detours.h"


//-----------------------------------------------
//                 ȫ�ֱ���
//-----------------------------------------------
static bool g_ShowMenu = true;          // ���Ʋ˵���ʾ
static HWND g_hWnd = nullptr;           // ��Ϸ���ھ��
static WNDPROC g_OriginalWndProc = nullptr; // ԭʼ���ڹ���
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



// ԭʼ��������
using wglSwapBuffers_t = BOOL(WINAPI*)(HDC);
wglSwapBuffers_t OriginalwglSwapBuffers = nullptr;

//-----------------------------------------------
//               �Զ��崰�ڹ���
//-----------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // �������¼����ݸ� ImGui
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
        return true; // ImGui �Ѵ�����¼�
    }

    // �����¼�����Ϸԭʼ���ڹ���
    return CallWindowProc(g_OriginalWndProc, hWnd, msg, wParam, lParam);
}

//-----------------------------------------------
//               ImGui ��ʼ��
//-----------------------------------------------
void InitImGui(HDC hdc) {
    // ��ȡ���ھ��
    g_hWnd = WindowFromDC(hdc);

    // ��ʼ�� ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // ���ü��̿���

    // ���÷��
    ImGui::StyleColorsDark();

    // ��ʼ�� Win32 �� OpenGL ���
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplOpenGL3_Init();

    // ����ԭʼ���ڹ��̣����滻Ϊ�Զ������
    g_OriginalWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
}

//-----------------------------------------------
//              Hook ��� wglSwapBuffers
//-----------------------------------------------
BOOL WINAPI HookedwglSwapBuffers(HDC hdc) {
    // �״ε���ʱ��ʼ�� ImGui
    static bool isInitialized = false;
    if (!isInitialized) {
        InitImGui(hdc);
        isInitialized = true;
    }

    // ��ʼ��֡

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // ���Ʋ˵�
    if (g_ShowMenu) {
        ImGui::Begin("game", &g_ShowMenu);
        ImGui::Text("hello");
        if (ImGui::Button("EXIT")) {
            g_ShowMenu = false;
        }
        ImGui::End();
    }

    // ��Ⱦ ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // ����ԭʼ������ɻ���������
    return OriginalwglSwapBuffers(hdc);
}

//-----------------------------------------------
//               Hook ��ʼ����ж��
//-----------------------------------------------
void InitHook() {
    // ��ȡԭʼ wglSwapBuffers ��ַ
    HMODULE hOpenGL = GetModuleHandleA("opengl32.dll");
    OriginalwglSwapBuffers = (wglSwapBuffers_t)GetProcAddress(hOpenGL, "wglSwapBuffers");

    // ʹ�� Detours ���� Hook
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)OriginalwglSwapBuffers, HookedwglSwapBuffers);
    DetourTransactionCommit();
}

void Unhook() {
    // �ָ�ԭʼ����
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)OriginalwglSwapBuffers, HookedwglSwapBuffers);
    DetourTransactionCommit();

    // ���� ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // �ָ����ڹ���
    SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)g_OriginalWndProc);
}