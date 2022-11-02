#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <shlwapi.h>

#include "logic.h"

#define UNINITIALIZED 0xFFFFFFFF
#define TARGET_EXE L"GenerationZero_F.exe"
//#define TARGET_EXE L"explorer.exe"

BYTE * g_baseAddress=0;
DWORD g_process_id;

void display_show(TCHAR *msg){
	MessageBox
	(	NULL //   [in, optional] HWND    hWnd
	,	msg //   [in, optional] LPCTSTR lpText
	,	L"Error" // [in, optional] LPCTSTR lpCaption
	,	MB_OK // [in]           UINT    uType
	);
}

int show_error_return_false( TCHAR* msg){
	display_show(msg);
	return 255;
}
BYTE * show_error_return_BS( TCHAR *msg ){
	display_show(msg);
	return (BYTE *)UNINITIALIZED;
}

BYTE * find_process_base_address(DWORD processID_) 
{
	HANDLE moduleSnapshotHandle_ = INVALID_HANDLE_VALUE;
	MODULEENTRY32 moduleEntry_;
	DWORD  processBaseAddress_   = UNINITIALIZED;

/* Take snapshot of all the modules in the process */
	moduleSnapshotHandle_ = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, processID_ );

/* Snapshot failed */
	if( moduleSnapshotHandle_ == INVALID_HANDLE_VALUE )
		return show_error_return_BS( L"Module Snapshot error" );

/* Size the structure before usage */
	moduleEntry_.dwSize = sizeof( MODULEENTRY32 );

/* Retrieve information about the first module */
	if( !Module32First( moduleSnapshotHandle_, &moduleEntry_ ) ) {
		CloseHandle( moduleSnapshotHandle_ );    
		return show_error_return_BS(L"Module32First");
	}

/* Find base address */
	while(processBaseAddress_ == UNINITIALIZED) {
/* Find module of the executable */
		do {
/* Compare the name of the process to the one we want */
			if( StrStrIW(moduleEntry_.szModule, TARGET_EXE) )
/* Save the processID and break out */
				return moduleEntry_.modBaseAddr;
		} while( Module32Next( moduleSnapshotHandle_, &moduleEntry_ ) );

		return show_error_return_BS (L"base address not found"); 
	}
	return show_error_return_BS (L"TOTAL failure"); 
}


int is_target_loaded() {
	PROCESSENTRY32 pe32;
	HANDLE hProcess;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
		return show_error_return_false(L"Error in CreateToolhelp32Snapshot");

	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( !Process32First( hProcessSnap, &pe32 ) ) {
		CloseHandle( hProcessSnap );
		return show_error_return_false (L"Error in Process32First");
	}

	do{
		hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
		if(StrStrIW(TARGET_EXE, pe32.szExeFile)) {
			g_baseAddress = find_process_base_address( pe32.th32ProcessID );
//			TCHAR buff[100];
//			wsprintf(buff, L"base address `%x`", g_baseAddress);
//			display_show(buff);
			g_process_id =  pe32.th32ProcessID;
//			wsprintf(buff, L"process id `%d`", g_process_id);
//			display_show(buff);
			CloseHandle(hProcess);
			return 1;
		}
		CloseHandle(hProcess);
	} while( Process32Next( hProcessSnap, &pe32 ) );

	return 0;
}

BOOL do_cheat_ammo(int on_off){
	char	nop_code[] = "\x90\x90\x90" // nop nop nop
	,	original_code[]="\x89\x4f\x30" // mov     dword ptr [rdi+30h],ecx (move the total amount of bullets in this place)
	,	check_buffer[]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	,	*memory_contents=original_code
	,	*cheat_contents=nop_code
	;
	LPVOID	lp_game_memory_address= 0x0;
	HANDLE	hProcess = 0;
	BOOL	b_res;
	SIZE_T  NumberOfBytesWritten
	,	NumberOfBytesRead
	,	num_bytes_to_write
	;

	if(0==on_off) { // ZERO=REMOVE CHEAT
		memory_contents=nop_code; // 1 INSTALL CHEAT
		cheat_contents=original_code;
	}

	/*
38> u GenerationZero_F+0x66cf5b
GenerationZero_F+0x66cf5b:
00007ff7`d4cbcf5b 894f30          mov     dword ptr [rdi+30h],ecx <-- HERE
00007ff7`d4cbcf5e 4584c9          test    r9b,r9b
00007ff7`d4cbcf61 0f84ce000000    je      GenerationZero_F+0x66d035 (00007ff7`d4cbd035)
00007ff7`d4cbcf67 48896c2458      mov     qword ptr [rsp+58h],rbp
00007ff7`d4cbcf6c 498b28          mov     rbp,qword ptr [r8]
00007ff7`d4cbcf6f 837d3000        cmp     dword ptr [rbp+30h],0
00007ff7`d4cbcf73 0f85b7000000    jne     GenerationZero_F+0x66d030 (00007ff7`d4cbd030)
00007ff7`d4cbcf79 8b4d2c          mov     ecx,dword ptr [rbp+2Ch]
	*/
	lp_game_memory_address= g_baseAddress+0x66cf5b;
	hProcess = OpenProcess
	(	STANDARD_RIGHTS_REQUIRED | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE
	,	FALSE
	,	g_process_id
	);
	if(NULL==hProcess)
		return show_error_return_false(L"Error in OpenProcess");
	b_res=ReadProcessMemory
	(	hProcess // [in]  HANDLE  hProcess
	,	lp_game_memory_address // [in]  LPCVOID lpBaseAddress
	,	check_buffer // [out] LPVOID  lpBuffer
	,	3 // [in]  SIZE_T  nSize
	,	&NumberOfBytesRead // [out] SIZE_T  *lpNumberOfBytesRead
	);
	if(0 == b_res) {
		CloseHandle(hProcess);
		return show_error_return_false(L"Error reading memory");
	}
	if(	memory_contents[0]!=check_buffer[0]
	||	memory_contents[1]!=check_buffer[1]
	||	memory_contents[2]!=check_buffer[2]
	) {
		CloseHandle(hProcess);
		return show_error_return_false(L"Original memory content does not match");
	}
	b_res = WriteProcessMemory
	(	hProcess //  [in]  HANDLE  hProcess
	,	lp_game_memory_address // [in]  LPVOID  lpBaseAddress
	,	(LPCVOID)cheat_contents // [in]  LPCVOID lpBuffer
	,	3 //[in]  SIZE_T  nSize
	,	&NumberOfBytesWritten // [out] SIZE_T *lpNumberOfBytesWritten
	);
	if(0==b_res) {
		CloseHandle(hProcess);
		return show_error_return_false(L"Error writing memory");
	}
	CloseHandle(hProcess);
	return b_res;
}
BOOL do_cheat_lives(int on_off){
	char	nop_code[] = "\x90\x90\x90" // nop nop nop
	,	original_code[]="\x2b\xd3" // sub     edx,ebx (subctract damage (ebx) from total lives (edx))
	,	check_buffer[]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	,	*memory_contents=original_code
	,	*cheat_contents=nop_code
	;
	LPVOID	lp_game_memory_address= 0x0;
	HANDLE	hProcess = 0;
	BOOL	b_res;
	SIZE_T  NumberOfBytesWritten
	,	NumberOfBytesRead
	,	num_bytes_to_write
	;

	if(0==on_off) { // ZERO=REMOVE CHEAT
		memory_contents=nop_code; // 1 INSTALL CHEAT
		cheat_contents=original_code;
	}

	/*
GenerationZero_F+0x90fdb2:
00007ff7`821afdb2 2bd3            sub     edx,ebx <-------------------- HERE
00007ff7`821afdb4 488b01          mov     rax,qword ptr [rcx]
00007ff7`821afdb7 410f48d0        cmovs   edx,r8d
00007ff7`821afdbb 4533c9          xor     r9d,r9d
00007ff7`821afdbe 4c8bc6          mov     r8,rsi
00007ff7`821afdc1 488b5c2430      mov     rbx,qword ptr [rsp+30h]
00007ff7`821afdc6 488b742438      mov     rsi,qword ptr [rsp+38h]
00007ff7`821afdcb 4883c420        add     rsp,20h
	*/
	lp_game_memory_address= g_baseAddress+0x90fdb2;
	hProcess = OpenProcess
	(	STANDARD_RIGHTS_REQUIRED | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE
	,	FALSE
	,	g_process_id
	);
	if(NULL==hProcess)
		return show_error_return_false(L"Error in OpenProcess");
	b_res=ReadProcessMemory
	(	hProcess // [in]  HANDLE  hProcess
	,	lp_game_memory_address // [in]  LPCVOID lpBaseAddress
	,	check_buffer // [out] LPVOID  lpBuffer
	,	2 // [in]  SIZE_T  nSize
	,	&NumberOfBytesRead // [out] SIZE_T  *lpNumberOfBytesRead
	);
	if(0 == b_res) {
		CloseHandle(hProcess);
		return show_error_return_false(L"Error reading memory");
	}
	if(	memory_contents[0]!=check_buffer[0]
	||	memory_contents[1]!=check_buffer[1]
	) {
		CloseHandle(hProcess);
		return show_error_return_false(L"Original memory content does not match");
	}
	b_res = WriteProcessMemory
	(	hProcess //  [in]  HANDLE  hProcess
	,	lp_game_memory_address // [in]  LPVOID  lpBaseAddress
	,	(LPCVOID)cheat_contents // [in]  LPCVOID lpBuffer
	,	2 //[in]  SIZE_T  nSize
	,	&NumberOfBytesWritten // [out] SIZE_T *lpNumberOfBytesWritten
	);
	if(0==b_res) {
		CloseHandle(hProcess);
		return show_error_return_false(L"Error writing memory");
	}
	CloseHandle(hProcess);
	return b_res;
}

BOOL do_cheat(char *what, int on_off){
	if(strstr("AMMO", what))
		return do_cheat_ammo(on_off);
	else if(strstr("LIVES", what))
		return do_cheat_lives(on_off);
	else
		return show_error_return_false (L"No such cheat");
}

