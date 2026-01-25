#include <windows.h>
#include <stdbool.h>
#include "definitions.h"
#include "logic.h"
#include "memscan.h"
#include "codecaves.h"

HINSTANCE g_dll_handle=NULL;
LPVOID g_pSharedMem = NULL;
HANDLE g_hMapFile = NULL;
WCHAR g_log_file_name[MAX_PATH]={0};

void init_jump_table(){
// Certain hacks need space to store the address to jump to.
// Looks like base address+0x450 is empty; let's put our junk there.
	DWORD	oldProtect;
	BYTE *jump_table=getBaseAddress()+0x450;
	VirtualProtect( jump_table , 32, PAGE_EXECUTE_READWRITE, &oldProtect);
// The '+4' below is to skip the C function prolog
	*(unsigned long long *)jump_table=(unsigned long long)flying_codecave+4; // base+0x450
	jump_table+=8;
	*(unsigned long long *)jump_table=(unsigned long long)super_speed_codecave+4; // base+0x458
	jump_table+=8;
	*(unsigned long long *)jump_table=(unsigned long long)super_speed_codecave+4; // base+0x460 ( unused)
	jump_table+=8;
	*(unsigned long long *)jump_table=(unsigned long long)move_clock_codecave+4; // base+0x468
	VirtualProtect( jump_table , 32, oldProtect, &oldProtect);
//The jump table now sits at base address+0x450. Its contents are:
// base+0x450 pointer to asm function flying_codecave
// base+0x458 super_speed_codecave
// base+0x460 this was easy_kill_codecave, no longer needed
// base+0x468 move_clock_codecave
}


DWORD WINAPI dll_thread(LPVOID param)
{
	int cheat_status[20]={0};
	file_log("%s : %d beginning housekeeping", __FILE__, __LINE__);
	if(0!=query_module_parameters()){
		return file_log("%s : %d Exiting", __FILE__, __LINE__);
	}

	init_jump_table();
	ScanCurrentProcessMemory();
	update_codecave_addresses();
	reset_acceleration_value();
	file_log("%s : %d housekeeping finished", __FILE__, __LINE__);
	while(true){
		if(GetAsyncKeyState(VK_NUMPAD0 //0x30 	is 0 key
		) ){
			FreeLibraryAndExitThread(g_dll_handle, 0);
			return 0; // exit thread; not really necessary
		}
		if(GetAsyncKeyState(VK_NUMPAD1 //0x31 	1 key
		) ){
			cheat_status[INFINITE_AMMO]=!cheat_status[INFINITE_AMMO];
			cheat_status[INFINITE_AMMO_2]=!cheat_status[INFINITE_AMMO_2];
			perform_action(INFINITE_AMMO, cheat_status[INFINITE_AMMO]);
			perform_action(INFINITE_AMMO_2, cheat_status[INFINITE_AMMO_2]);
		}
		if(GetAsyncKeyState(VK_NUMPAD2 //0x32 	2 key
		) ){
			cheat_status[INFINITE_LIFE]=!cheat_status[INFINITE_LIFE];
			perform_action(INFINITE_LIFE, cheat_status[INFINITE_LIFE]);
		}
		if(GetAsyncKeyState(VK_NUMPAD3
		) ){
			cheat_status[INFINITE_STAMINA]=!cheat_status[INFINITE_STAMINA];
			perform_action(INFINITE_STAMINA, cheat_status[INFINITE_STAMINA]);
		}
		if(GetAsyncKeyState(VK_NUMPAD4
		) ){
			if( GetAsyncKeyState(VK_MENU) )	//	ALT key
				increase_pain();
			else if( GetAsyncKeyState(VK_CONTROL) )
				reset_pain();
			else {
				cheat_status[LETS_KILL]=!cheat_status[LETS_KILL];
				perform_action(LETS_KILL, cheat_status[LETS_KILL]);
			}
		}
		if(GetAsyncKeyState(VK_NUMPAD5
		) ){
			cheat_status[ZERO_WEIGHT]=!cheat_status[ZERO_WEIGHT];
			perform_action(ZERO_WEIGHT, cheat_status[ZERO_WEIGHT]);
		}
		if(GetAsyncKeyState(VK_NUMPAD6
		) ){
			if( GetAsyncKeyState(VK_MENU) )	//	ALT key
				increase_acceleration_value();
			else if( GetAsyncKeyState(VK_CONTROL) )
				reset_acceleration_value();
			else {
				cheat_status[LETS_FLY]=!cheat_status[LETS_FLY];
				perform_action(LETS_FLY, cheat_status[LETS_FLY]);
			}
		}
		if(GetAsyncKeyState(VK_NUMPAD7) ){
			if( GetAsyncKeyState(VK_MENU) )	//	ALT key
				increase_speed();
			else if( GetAsyncKeyState(VK_CONTROL) )
				reset_speed();
			else {
				cheat_status[LETS_RUN]=!cheat_status[LETS_RUN];
				perform_action(LETS_RUN, cheat_status[LETS_RUN]);
			}
		}
		if(GetAsyncKeyState(VK_NUMPAD8
		) ){
			if( GetAsyncKeyState(VK_MENU) )	//	ALT key
				increase_time_gap();
			else if( GetAsyncKeyState(VK_CONTROL) )
				reset_time_gap();
			else {
				cheat_status[LETS_TICK]=!cheat_status[LETS_TICK];
				perform_action(LETS_TICK, cheat_status[LETS_TICK]);
			}
		}
		Sleep(100);
	}
	return 0;
}

BOOL ReadParametersFromSharedMemory() {
	g_hMapFile = OpenFileMapping
	(	FILE_MAP_READ
	,	FALSE
	,	"Global\\MyDLLParams"
	);

	if (g_hMapFile) {
		g_pSharedMem = MapViewOfFile
		(	g_hMapFile
		,	FILE_MAP_READ
		,	0
		,	0
		,	0
		);

		if (g_pSharedMem) {
			DLL_PARAMS* params = (DLL_PARAMS*)g_pSharedMem;
			wcscpy(g_log_file_name, params->log_file_path);
			return TRUE;
		}
	}
	return FALSE;
}


BOOL WINAPI DllMain
(	HINSTANCE hinstDLL // handle to DLL module
,	DWORD fdwReason    // reason for calling function
,	LPVOID lpvReserved // reserved
)
{
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			g_dll_handle=hinstDLL;
			if(!ReadParametersFromSharedMemory())
				wcscpy(L"c:\\log.txt", g_log_file_name); // default
			
			HANDLE h = CreateThread
			(	(LPSECURITY_ATTRIBUTES)0		// [in, optional]  lpThreadAttributes
			,	(SIZE_T)0				// [in] dwStackSize
			,	(LPTHREAD_START_ROUTINE)dll_thread	// [in] lpStartAddress
			,	(HINSTANCE)hinstDLL			// [in, optional]  __drv_aliasesMem LPVOID lpParameter
			,	(DWORD)0				// [in]            DWORD                   dwCreationFlags
			,	(LPDWORD)0				// [out, optional] LPDWORD                 lpThreadId
			);
			if(h)
				CloseHandle(h);
			return TRUE;  // Successful DLL_PROCESS_ATTACH
		break;


		case DLL_PROCESS_DETACH: // Cleanup
			if (g_pSharedMem) {
				UnmapViewOfFile(g_pSharedMem);
				g_pSharedMem = NULL;
			}
			if (g_hMapFile) {
				CloseHandle(g_hMapFile);
				g_hMapFile = NULL;
			}
			return TRUE;  // Successful DLL_PROCESS_DETACH
		break;
		}
	return TRUE;
}

