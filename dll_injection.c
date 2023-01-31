#include <windows.h>
#include <stdbool.h>
#include "definitions.h"
#include "logic.h"

void init_jump_table(){
// Certain hacks need space to store the address to jump to.
// Looks like base address+0x450 is empty, let's put our junk there.
	DWORD	oldProtect;
	BYTE *jump_table=get_base_address()+0x450;
	VirtualProtect( jump_table , 32, PAGE_EXECUTE_READWRITE, &oldProtect);
	*(unsigned long long *)jump_table=(unsigned long long)flying_codecave+4; // base+0x450
	jump_table+=8;
	*(unsigned long long *)jump_table=(unsigned long long)super_speed_codecave+4; // base+0x458
	jump_table+=8;
	*(unsigned long long *)jump_table=(unsigned long long)easy_kill_codecave+4; // base+0x460
	jump_table+=8;
	*(unsigned long long *)jump_table=(unsigned long long)move_clock_codecave+0x15; // base+0x468; 0x15 is because there is a local float
	VirtualProtect( jump_table , 32, oldProtect, &oldProtect);
}


DWORD WINAPI dll_thread(LPVOID param)
{
	int cheat_status[10]={0};
	init_jump_table();
	reset_acceleration_value();
	while(true){
		if(GetAsyncKeyState(VK_NUMPAD0) ){
			return 0; // exit thread
		}
		if(GetAsyncKeyState(VK_NUMPAD1) ){
			cheat_status[INFINITE_AMMO]=!cheat_status[INFINITE_AMMO];
			cheat_status[NO_RECHARGE]=!cheat_status[NO_RECHARGE];
			perform_action(INFINITE_AMMO, cheat_status[INFINITE_AMMO]);
			perform_action(NO_RECHARGE, cheat_status[NO_RECHARGE]);
		}
		if(GetAsyncKeyState(VK_NUMPAD2) ){
			cheat_status[INFINITE_LIFE]=!cheat_status[INFINITE_LIFE];
			perform_action(INFINITE_LIFE, cheat_status[INFINITE_LIFE]);
		}
		if(GetAsyncKeyState(VK_NUMPAD3) ){
			cheat_status[INFINITE_STAMINA]=!cheat_status[INFINITE_STAMINA];
			perform_action(INFINITE_STAMINA, cheat_status[INFINITE_STAMINA]);
		}
		if(GetAsyncKeyState(VK_NUMPAD4) ){
			if( GetAsyncKeyState(VK_MENU) )	//	ALT key
				increase_pain();
			else if( GetAsyncKeyState(VK_CONTROL) )
				reset_pain();
			else {
				cheat_status[LETS_KILL]=!cheat_status[LETS_KILL];
				perform_action(LETS_KILL, cheat_status[LETS_KILL]);
			}
		}
		if(GetAsyncKeyState(VK_NUMPAD5) ){
			cheat_status[ZERO_WEIGHT]=!cheat_status[ZERO_WEIGHT];
			perform_action(ZERO_WEIGHT, cheat_status[ZERO_WEIGHT]);
		}
		if(GetAsyncKeyState(VK_NUMPAD6) ){
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
		if(GetAsyncKeyState(VK_NUMPAD8) ){
			cheat_status[LETS_TICK]=!cheat_status[LETS_TICK];
			perform_action(LETS_TICK, cheat_status[LETS_TICK]);
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

