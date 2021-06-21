// CodeInjectorSimple.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <comdef.h> 

#include <tchar.h>

typedef int(__stdcall* __MessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
typedef int(__stdcall* __MsgITD)(const unsigned long long, const unsigned long long);
class injData
{
public:
    char message[256];
    char title[256];
    DWORD paMessageBoxA;
};
DWORD getModuleAddress(const char* procName, DWORD pID)
{
    DWORD id = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);

    if (hSnap != INVALID_HANDLE_VALUE)
    {

        MODULEENTRY32 entry = { 0 };
        entry.dwSize = sizeof(entry);

        if (Module32First(hSnap, &entry))
        {
            do
            {
                _bstr_t b(entry.szModule);
                const char* exeFIle = b;
                if (strcmp(procName, exeFIle)==0)
                {
                    id = (DWORD)entry.modBaseAddr;
                    break;
                }


            } while (Module32Next(hSnap, &entry));

        }
    }

    CloseHandle(hSnap);
    return id;
}

DWORD getProcessId(const char* procName)
{
    DWORD id=0;
  HANDLE hSnap=  CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

  if (hSnap != INVALID_HANDLE_VALUE)
  {

      PROCESSENTRY32 entry;
      entry.dwSize = sizeof(PROCESSENTRY32);
      if (Process32First(hSnap, &entry))
      {
          do
          {
              _bstr_t b(entry.szExeFile);
              const char* exeFIle = b;
               if (!_stricmp(exeFIle, procName))
                {
                  
                   id = entry.th32ProcessID;
                    break;
                }


          }  while (Process32Next(hSnap, &entry));

      }
  }

  CloseHandle(hSnap);
    return id;
}

DWORD __stdcall remoteThread(injData* iData)
{
    __MessageBoxA MsgBox = (__MessageBoxA)iData->paMessageBoxA;
    MsgBox(NULL, iData->message, iData->title, MB_ICONINFORMATION);
    return EXIT_SUCCESS;
}



class iDt
{

public:
    unsigned long long a;
    unsigned long long b;
    DWORD paMsg;
};
DWORD  remoteFun(iDt *idt)
{
    __MsgITD mb = (__MsgITD)idt->paMsg;
    mb(idt->a, idt->b);
    return EXIT_SUCCESS;
}
int main()
{

    iDt dt;
    ZeroMemory(&dt, sizeof(iDt));
    dt.a = 132234;
    dt.b = 7000;
    const char* port = "C:\\Users\\Office_1\\source\\repos\\InjectTestDLL\\Release\\InjectTestDLL.dll";
    //C:\\Users\\Office_1\\source\\repos\\InjectTestDLL\\Release\\InjectTestDLL
    //"C:\\Users\\Office_1\\source\\repos\\InjectTestDLL\\x64\\Release\\InjectTestDLL.dll"
    DWORD procID = getProcessId("CrackTestPr.exe");
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);
    void* premoteThread = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    //ULONG addr = 0x0007FF67DC21DA0;
    //ULONG mID = getModuleAddress("CrackTestPr.exe", procID);
    //std::cout << procID << "success\n";
  
    if (WriteProcessMemory(hProc, premoteThread, port, strlen(port) + 1, 0))
        std::cout << "success\n";
    HANDLE hthread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, premoteThread, 0, 0);
    CloseHandle(hthread);
    CloseHandle(hProc);





    //==============================================================================================
    /*
   HINSTANCE userLibrary =LoadLibraryA(port);
   dt.paMsg = (DWORD)GetProcAddress(userLibrary, "fibonacci_init"); //PrintMesage
   FreeLibrary(userLibrary);

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, getProcessId("ConsoleApplication2.exe"));
    LPVOID premoteThread = VirtualAllocEx(hProc, NULL, sizeof(iDt), MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
     WriteProcessMemory(hProc, premoteThread, (LPVOID)remoteFun, sizeof(iDt), 0);
    // HANDLE hthread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)premoteThread, 0, 0, 0);
     iDt *fData= (iDt*)VirtualAllocEx(hProc, NULL, sizeof(iDt), MEM_COMMIT , PAGE_READWRITE);
      WriteProcessMemory(hProc, fData, &dt, sizeof(iDt), NULL);
      HANDLE hthread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)premoteThread, fData, 0, 0);
      CloseHandle(hthread);
     VirtualFreeEx(hProc, premoteThread, sizeof(iDt), MEM_RELEASE);
      CloseHandle(hProc);
      */

//==============================================================================================
    //injData iData;
    //ZeroMemory(&iData, sizeof(injData));
    //strcpy_s(iData.message, "I called from RP");
    //strcpy_s(iData.title, "Hello from RP");

    //const char* port = "user32.dll";
    //size_t size = strlen(port) + 1;
    //wchar_t* portName = new wchar_t[size];
    //size_t outSize;
    //mbstowcs_s(&outSize, portName, size, port, size - 1);


    //HINSTANCE userLibrary =LoadLibrary(portName);
    //iData.paMessageBoxA = (DWORD)GetProcAddress(userLibrary, "MessageBoxA");
    //FreeLibrary(userLibrary);

    //HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, getProcessId("WindowsProject1.exe"));
    //LPVOID premoteThread = VirtualAllocEx(hProc, NULL, sizeof(injData), MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    //WriteProcessMemory(hProc, premoteThread, (LPVOID)remoteThread, sizeof(injData), 0);

    //injData *pData= (injData*)VirtualAllocEx(hProc, NULL, sizeof(injData), MEM_COMMIT , PAGE_READWRITE);
    //WriteProcessMemory(hProc, pData, &iData, sizeof(injData), NULL);

    //HANDLE hthread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)premoteThread, pData, 0, 0);

    //CloseHandle(hthread);
    //VirtualFreeEx(hProc, premoteThread, sizeof(injData), MEM_RELEASE);
    //CloseHandle(hProc);
}

