#include <windows.h>
#include <stdbool.h>
#include "definitions.h"
#include "logic.h"


DWORD WINAPI dll_thread(LPVOID param)
{
	MessageBoxA(NULL, "Thread started" , "Info" , MB_OK);
	int cheat_status[3]={0};
	while(true){
		if(GetAsyncKeyState(VK_NUMPAD1) & 0x8000){
			cheat_status[0]=!cheat_status[0];
			perform_action(CHEAT_AMMO, cheat_status[0]);
		}
		if(GetAsyncKeyState(VK_NUMPAD2) & 0x8000){
			cheat_status[1]=!cheat_status[1];
			perform_action(CHEAT_LIFE, cheat_status[1]);
		}
		if(GetAsyncKeyState(VK_NUMPAD3) & 0x8000){
			cheat_status[2]=!cheat_status[2];
			perform_action(CHEAT_NO_RECHARGE, cheat_status[2]);
		}
		Sleep(100);
	}
	return 0;
}

BOOL WINAPI DllMain
(	HINSTANCE hinstDLL // handle to DLL module
,	DWORD fdwReason    // reason for calling function
,	LPVOID lpvReserved // reserved
)
{
	if( DLL_PROCESS_ATTACH == fdwReason ) { 
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
			HANDLE h = CreateThread
			(	(LPSECURITY_ATTRIBUTES)0		// [in, optional]  lpThreadAttributes
			,	(SIZE_T)0				// [in] dwStackSize
			,	(LPTHREAD_START_ROUTINE)dll_thread	// [in] lpStartAddress
			,	(HINSTANCE)hinstDLL			// [in, optional]  __drv_aliasesMem LPVOID lpParameter
			,	(DWORD)0				// [in]            DWORD                   dwCreationFlags
			,	(LPDWORD)0				// [out, optional] LPDWORD                 lpThreadId
			);
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH
}

