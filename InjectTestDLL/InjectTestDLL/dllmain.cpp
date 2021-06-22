// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "InjectLib.h"
#include <iostream>

DWORD WINAPI myThread(HMODULE hModule)
{
    greenSquaresSpawn_Init(hModule, 4);
    //while true
    //{
        //Some hack logic.
    //}
    CloseHandle(hModule);
    return 0;
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:  
        CloseHandle(CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)myThread, hModule, NULL, nullptr));
    case DLL_THREAD_ATTACH: 
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    
    return TRUE;
}

