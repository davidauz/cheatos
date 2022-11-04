#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <shlwapi.h>
#include <stdbool.h>

#include "logic.h"
#include "definitions.h"

#define UNINITIALIZED 0xFFFFFFFF
#define TARGET_EXE L"GenerationZero_F.exe"
//#define TARGET_EXE L"explorer.exe"

BYTE * g_baseAddress=0;
DWORD g_process_id=0;

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

// Take a snapshot of all the modules in the process
	moduleSnapshotHandle_ = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, processID_ );
	if( moduleSnapshotHandle_ == INVALID_HANDLE_VALUE )
		return show_error_return_BS( L"Module Snapshot error" );

// structure size
	moduleEntry_.dwSize = sizeof( MODULEENTRY32 );

// the first module
	if( !Module32First( moduleSnapshotHandle_, &moduleEntry_ ) ) {
		CloseHandle( moduleSnapshotHandle_ );    
		return show_error_return_BS(L"Error in Module32First");
	}

// Find base address
	while(processBaseAddress_ == UNINITIALIZED) {
// Find module of the executable
		do {
			if( StrStrIW(moduleEntry_.szModule, TARGET_EXE) )
				return moduleEntry_.modBaseAddr;
		} while( Module32Next( moduleSnapshotHandle_, &moduleEntry_ ) );

		return show_error_return_BS (L"base address not found"); 
	}
	return show_error_return_BS (L"TOTAL failure"); 
}




void find_process_id(){
	PROCESSENTRY32 pe32;
	HANDLE hProcess;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	g_baseAddress = 0;
	g_process_id = 0;
	if( hProcessSnap == INVALID_HANDLE_VALUE ){
		show_error_return_BS(L"Error in CreateToolhelp32Snapshot");
		return;
	}

	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( !Process32First( hProcessSnap, &pe32 ) ) {
		CloseHandle( hProcessSnap );
		show_error_return_BS (L"Error in Process32First");
		return;
	}

	do{
		hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
		if(StrStrIW(TARGET_EXE, pe32.szExeFile)) {
			g_baseAddress = find_process_base_address( pe32.th32ProcessID );
			g_process_id =  pe32.th32ProcessID;
			CloseHandle(hProcess);
			break;
		}
		CloseHandle(hProcess);
	} while( Process32Next( hProcessSnap, &pe32 ) );

	if(0==g_process_id)
		show_error_return_false (L"Could not find process");

	return;
}

int perform_action(int cheat_id, bool on_off) {
	struct cheat_definition *p_definition=&definitions[cheat_id];
	char	*nop_code = p_definition->cheat_code
	,	*original_code = p_definition->original_code
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

	if(0==g_process_id)
		find_process_id();
	if(0==g_process_id)
		return show_error_return_false(L"Error in OpenProcess");

	lp_game_memory_address= g_baseAddress+p_definition->memory_address;

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
	,	p_definition->cheat_num_bytes // [in]  SIZE_T  nSize
	,	&NumberOfBytesRead // [out] SIZE_T  *lpNumberOfBytesRead
	);
	if(0 == b_res) {
		CloseHandle(hProcess);
		return show_error_return_false(L"Error reading memory");
	}

	if(NumberOfBytesRead != p_definition->cheat_num_bytes) {
		CloseHandle(hProcess);
		return show_error_return_false(L"Size mismatch reading memory");
	}
	for(int i=0; i<p_definition->cheat_num_bytes; i++)
		if(memory_contents[i]!=check_buffer[i]) {
			CloseHandle(hProcess);
			return show_error_return_false(L"Original memory content does not match");
		}
	b_res = WriteProcessMemory
	(	hProcess //  [in]  HANDLE  hProcess
	,	lp_game_memory_address // [in]  LPVOID  lpBaseAddress
	,	(LPCVOID)cheat_contents // [in]  LPCVOID lpBuffer
	,	p_definition->cheat_num_bytes //[in]  SIZE_T  nSize
	,	&NumberOfBytesWritten // [out] SIZE_T *lpNumberOfBytesWritten
	);
	if(0==b_res) {
		CloseHandle(hProcess);
		return show_error_return_false(L"Error writing memory");
	}
	if(NumberOfBytesWritten != p_definition->cheat_num_bytes) {
		CloseHandle(hProcess);
		return show_error_return_false(L"Size mismatch reading memory");
	}
	CloseHandle(hProcess);
	return 1;
}


