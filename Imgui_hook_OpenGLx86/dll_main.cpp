// dllmain.cpp
#include "Hook.hpp"
//-----------------------------------------------
//               DLL Èë¿Úµã
//-----------------------------------------------
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InitHook, 0, 0, 0);
        break;
    case DLL_PROCESS_DETACH:
        Unhook();
        break;
    }
    return TRUE;
}