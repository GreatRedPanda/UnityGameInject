#include "pch.h"
#include "InjectLib.h"
#include <iostream>

typedef void (*MyProc)();
const unsigned char OP_JMP = 0xE9;
const SIZE_T SIZE_PATCH = 5;

uintptr_t moduleAssembly = (uintptr_t)GetModuleHandleW(L"GameAssembly.dll");
void* mySpawner = nullptr;
typedef  float (*tgetY) (void* Spawner);
typedef  void (*tUpdate) (void* Spawner);
typedef void (*tspawnEnemy)(void* Spawner, float x, float y);
tspawnEnemy ospawnEnemy = nullptr;
typedef void (*tspawnBonus)(void* Spawner);
tspawnBonus ospawnBonus = (tspawnBonus)(moduleAssembly + 0x3D8F50); //RVA 



tgetY ogetY = nullptr;
tUpdate oUpdate = nullptr;

void hUpdate(void* Spawner) //как входной параметр приходит указатель на Spawner
{
	mySpawner = Spawner;

	if (mySpawner)
	{

		PBYTE foo = reinterpret_cast<PBYTE>((moduleAssembly + 0x3D8F50)); //число 16 преобразуетс¤ в целочисленный указатель на начало метода spawnBonus
		reinterpret_cast<tspawnBonus>(foo)(mySpawner);
		//ƒл¤ статических методов не надо знать указатель на mySpawner
		//PBYTE spawnRef = reinterpret_cast<PBYTE>((moduleAssembly + 0x3D9060));
		//reinterpret_cast<MyProc>(spawnRef)();
	}
	//oUpdate(Spawner);
}
float hgetY(void* Spawner)
{
	mySpawner = Spawner;
	return 4.0f;
}
float hgetY2()
{
	return 0.0f;
}

void  hspawnBonus(void* Spawner)
{
	mySpawner = Spawner;
	//Call original function and return correct value for when its called from the game.
	  //ospawnBonus(Spawner);
}

//void*  hspawnEnemy(void* Spawner, float x, float y)
//{
//	mySpawner = Spawner;
//	//Call original function and return correct value for when its called from the game.
//	return  ospawnEnemy(Spawner,1.0f,4.0f);
//}


void Hook(void* src, void* dst, unsigned int len)
{
	if (len < 5) return;

	DWORD curProtection;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

	//затирание пустыми инструкци¤ми
	// заполнение пам¤ти  на указанную длину  числом 0x90
	memset(src, 0x90, len);

	uintptr_t relativeAddress = (uintptr_t)dst - (uintptr_t)src - 5;

	//src - адрес 
	// *указатель - по этому адресу берут значение
	//в ¤чейку начала функции записывают команду относительного прыжка
	*(BYTE*)src = 0xE9;
	//берут адрес со смещением на 1 инструкцию. ѕо новому адресу записывают в него значение дл¤ прыжка
	*(uintptr_t*)((uintptr_t)src + 1) = relativeAddress;

	DWORD temp;
	VirtualProtect(src, len, curProtection, &temp);


	//*(BYTE*)src = 0xE9;

	//*(uintptr_t*)((BYTE*)src + 1) = relativeAddress;
	//VirtualProtect(src, len, curProtection, &curProtection);
}

BYTE* TrampHook(void* src, void* dst, unsigned int len)
{
	if (len < 5) return 0;

	//Create Gateway.
	BYTE* gateway = (BYTE*)VirtualAlloc(0, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	//Write stolen bytes.
	//из src копирует в gateway указанное количество байт len
	memcpy_s(gateway, len, src, len);

	//Get the gateway to destination address.
	uintptr_t gatewayRelativeAddr = (BYTE*)src - gateway - 5;
	*(gateway + len) = 0xE9;

	//Write address of gateway to jmp.
	*(uintptr_t*)((uintptr_t)gateway + len + 1) = gatewayRelativeAddr;

	//Perform HOOK.
	Hook(src, dst, len);
	return gateway;
}

void greenSquaresSpawn_Init(
    HMODULE module,
    const unsigned long long b)
{

	oUpdate = (tUpdate)(moduleAssembly + 0x3D8F00);
	 TrampHook(oUpdate, hUpdate, 6);
	while (!(GetAsyncKeyState(VK_ESCAPE) & 1))
	{
	
		if (!mySpawner) continue;
		//Lots of if(GetAsyncKeyState(VIRTUALKEYCODE) & 1) to activate or deactivate other hacks.
		Sleep(1);
	}
}