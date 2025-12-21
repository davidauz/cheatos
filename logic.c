#include <windows.h>
#include <inttypes.h>
#include <shlwapi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <tlhelp32.h>

#include "logic.h"
#include "definitions.h"
#include "sounds\down.c"
#include "sounds\up.c"

#define UNINITIALIZED 0xFFFFFFFF
// #define TARGET_EXE "GenerationZero_F.exe"
#define TARGET_EXE "Notepad.exe" // TESTS

BYTE * g_baseAddress=0;
DWORD g_process_id=0;
float	g_y_acceleration
,	g_speed=40
,	g_PAIN=50
,	l_TICK=0.01
;


int file_log(char* format, ...){
	char buf[255]
	;
	DWORD	dwBytesWritten;
	va_list argptr;
	va_start(argptr, format);
	vsprintf(buf, format, argptr);
	strcat(buf, "\n");
	va_end(argptr);
	HANDLE report_file_handle;
	report_file_handle = CreateFileW
	(	g_log_file_name	// file to write
	,	FILE_APPEND_DATA
	,	0		// do not share
	,	NULL		// default security
	,	OPEN_ALWAYS
	,	FILE_ATTRIBUTE_NORMAL// normal file
	,	NULL		// no attr. template
	);
	if(INVALID_HANDLE_VALUE==report_file_handle)
		return 255;
	WriteFile
	(	report_file_handle
	,	buf// data to write
	,	strlen(buf)	// number of bytes to write
	,	&dwBytesWritten	// number of bytes that were written
	,	NULL      // no overlapped structure
	);
	CloseHandle(report_file_handle);
	return 0;
}

void reset_pain(){
	g_PAIN=0;
	file_log( "%s:%d pain now `%f`", __FILE__, __LINE__, g_PAIN );
}
void increase_pain(){
	g_PAIN+=10;
	file_log( "%s:%d pain now `%f`", __FILE__, __LINE__, g_PAIN );
}

// GCC-style inline assembly syntax breakdown
void easy_kill_codecave(){
__asm__(
	"movss  %0,%%xmm2;"	// %0 is replaced with the memory reference to g_PAIN
	"mulss  0x20(%%rbx),%%xmm2;"
	"mulss  %%xmm0,%%xmm2;"
	"ret;"
	:			// No output operands (empty after first colon)
	: "m" (g_PAIN)		// Input operand: g_PAIN is in memory
);
//      |  |     └─── C variable/expression
//      |  └───────── Constraint ("m" = 'memory operand')
//      └──────────── Colon separating assembly template from inputs
}

void increase_time_gap(){
l_TICK+=0.01;
	file_log( "%s:%d TICK now `%f`", __FILE__, __LINE__, l_TICK );
}
void reset_time_gap() {
l_TICK=0.01;
	file_log( "%s:%d TICK now `%f`", __FILE__, __LINE__, l_TICK );
}
void move_clock_codecave (){
__asm__(
	"addss  %0,%%xmm0;"
	"movss  %%xmm0,0xe0(%%rcx);"
	"comiss %%xmm1,%%xmm0;"
	"ret;"
	:
	: "m" (l_TICK)
);
}

void super_speed_codecave(){
__asm__(
	"movss  0x1c(%%rsi),%%xmm6;"	//	put rsi+0x1c in xmm6 (original instruction)
	"mulss  %0,%%xmm6;"	//	multiply xmm6 by parameter
"exit:"
	"subss  0x18(%%rsi),%%xmm6;"	//	original instruction
	"mulss  %%xmm14,%%xmm6;"		//	original instruction
	"ret;"
	:
	: "m" (g_speed)
);
}

void flying_codecave(){
__asm__(
	"movss  0x44(%%rdi),%%xmm2;"	// put rdi+68 in xmm2
	"addss  %0,%%xmm2;"		// add g_y_acceleration to xmm2
	"movss  %%xmm2,0x44(%%rdi);"	// result goes to rdi+0x44
	"movaps %%xmm6,0x50(%%rdi);"	// original instruction
	"ret;"
	:
	: "m" (g_y_acceleration)
);
}

void reset_speed(){
	g_speed=0;
	file_log( "%s:%d SPEED now `%f`", __FILE__, __LINE__, g_speed );
}

void reset_acceleration_value(){
	g_y_acceleration=5;
	file_log( "%s:%d ACCELERATION now `%f`", __FILE__, __LINE__, g_y_acceleration );
}

void increase_speed(){
	g_speed+=10;
	file_log( "%s:%d SPEED now `%f`", __FILE__, __LINE__, g_speed );
}

void increase_acceleration_value(){
	g_y_acceleration+=10;
	file_log( "%s:%d ACCELERATION now `%f`", __FILE__, __LINE__, g_y_acceleration );
}


BYTE * find_process_base_address(DWORD processID_) 
{
	HANDLE moduleSnapshotHandle_ = INVALID_HANDLE_VALUE;
	MODULEENTRY32 moduleEntry_;
	DWORD  processBaseAddress_   = UNINITIALIZED;

// Take a snapshot of all the modules in the process
	moduleSnapshotHandle_ = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, processID_ );
	if( moduleSnapshotHandle_ == INVALID_HANDLE_VALUE )
		return file_log( "Module Snapshot error" ) ? NULL:NULL; // one liner

// structure size
	moduleEntry_.dwSize = sizeof( MODULEENTRY32 );

// the first module
	if( !Module32First( moduleSnapshotHandle_, &moduleEntry_ ) ) {
		CloseHandle( moduleSnapshotHandle_ );    
		file_log("Error in Module32First");
		return NULL;
	}

// Find base address
	while(processBaseAddress_ == UNINITIALIZED) {
// Find module of the executable
		do {
			if( strstr(moduleEntry_.szModule, TARGET_EXE) )
				return moduleEntry_.modBaseAddr;
		} while( Module32Next( moduleSnapshotHandle_, &moduleEntry_ ) );

		return file_log( "Base address not found" ) ? NULL:NULL;
	}
	return (file_log( "Total failure" ) ? NULL:NULL);
}




int find_process_id(){
	PROCESSENTRY32 pe32;
	HANDLE hProcess;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	g_baseAddress = 0;
	g_process_id = 0;
	if( hProcessSnap == INVALID_HANDLE_VALUE )
		return file_log("Error in CreateToolhelp32Snapshot")?0:0;

	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( !Process32First( hProcessSnap, &pe32 ) ) {
		CloseHandle( hProcessSnap );
		file_log("Error in Process32First");
		return 0;
	}

	do{
		hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
		if(strstr(TARGET_EXE, pe32.szExeFile)) {
			g_baseAddress = find_process_base_address( pe32.th32ProcessID );
			g_process_id =  pe32.th32ProcessID;
			file_log("L %d: process id is `%d`", __LINE__, g_process_id);
			CloseHandle(hProcess);
			break;
		}
		CloseHandle(hProcess);
	} while( Process32Next( hProcessSnap, &pe32 ) );

	return g_process_id;
}



BYTE *get_base_address(){
	if(0==g_baseAddress)
		find_process_id();
	return g_baseAddress;
}


int perform_dll_injection() {
	char	dll_name[] = "cheatos.dll"
	,	dll_path[MAX_PATH]={0}
	;

	SIZE_T  NumberOfBytesWritten;
	BOOL	b_res;
	if(0 == GetFullPathNameA
	(	dll_name // [in]  LPCSTR lpFileName,
	,	MAX_PATH // [in]  DWORD  nBufferLength,
	,	dll_path // [out] LPSTR  lpBuffer,
	,	NULL // [out] LPSTR  *lpFilePart
	))
		return file_log("Error in GetFullPathNameA")?FALSE:FALSE;
	int	n_dll_path_size=1+strlen(dll_path);
	HANDLE hProcess = OpenProcess
	(	STANDARD_RIGHTS_REQUIRED | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE
	,	FALSE
	,	g_process_id
	);
	if(NULL==hProcess)
		return file_log("Error in OpenProcess")?FALSE:FALSE;
	LPVOID p_dll_memory= VirtualAllocEx
	(	hProcess // [in]           HANDLE hProcess,
	,	NULL // [in, optional] LPVOID lpAddress,
	,	n_dll_path_size // [in]           SIZE_T dwSize,
	,	MEM_COMMIT|MEM_RESERVE // [in]           DWORD  flAllocationType,
	,	PAGE_READWRITE // [in]           DWORD  flProtect
	);
	b_res = WriteProcessMemory
	(	hProcess //  [in]  HANDLE  hProcess
	,	p_dll_memory // [in]  LPVOID  lpBaseAddress
	,	dll_path // [in]  LPCVOID lpBuffer
	,	n_dll_path_size //[in]  SIZE_T  nSize
	,	&NumberOfBytesWritten // [out] SIZE_T *lpNumberOfBytesWritten
	);
	if(0==b_res) {
		CloseHandle(hProcess);
		return file_log("Error writing memory")?FALSE:FALSE;
	}
	if(NumberOfBytesWritten != n_dll_path_size) {
		CloseHandle(hProcess);
		return file_log("Size mismatch reading memory")?FALSE:FALSE;
	}
	file_log("L %d: Creating thread", __LINE__);
	HANDLE dll_thread_handle = CreateRemoteThread
	(	 hProcess // [in]  HANDLE                 hProcess,
	,	 NULL // [in]  LPSECURITY_ATTRIBUTES  lpThreadAttributes,
	,	 (SIZE_T)NULL // [in]  SIZE_T                 dwStackSize,
	,	 (LPTHREAD_START_ROUTINE)LoadLibraryA// [in]  LPTHREAD_START_ROUTINE lpStartAddress,
	,	 p_dll_memory // [in]  LPVOID                 lpParameter,
	,	 (DWORD)0 // [in]  DWORD                  dwCreationFlags,
	,	 NULL // [out] LPDWORD                lpThreadId
	);
	WaitForSingleObject(dll_thread_handle, INFINITE);
	CloseHandle(dll_thread_handle);
	b_res=VirtualFreeEx
	(	hProcess // [in] HANDLE hProcess,
	,	dll_path // [in] LPVOID lpAddress,
	,	n_dll_path_size // [in] SIZE_T dwSize,
	,	MEM_RELEASE // [in] DWORD  dwFreeType
	);

	CloseHandle(hProcess);
	file_log("L %d: return", __LINE__);

	return 0;
}





int perform_action
(	int cheat_id
,	bool on_off
) {
	struct cheat_definition *p_definition=&definitions[cheat_id];
	char *snd=sound_UP;
	if(0==on_off)
		snd=sound_DOWN;
	PlaySound(snd, NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT); 

	if(0==g_process_id)
		find_process_id();
	if(0==g_process_id)
		return file_log("Error in find_process_id")?FALSE:FALSE;

	unsigned char	*nop_code = p_definition->cheat_code
	,	*original_code = p_definition->original_code
	,	check_buffer[50]={0}
	;
	BYTE	*lp_game_memory_address= 0x0
	,	*memory_contents=original_code
	,	*cheat_contents=nop_code
	;
	HANDLE	hProcess = 0;
	BOOL	b_res;
	SIZE_T  NumberOfBytesWritten
	,	NumberOfBytesRead
	,	num_bytes_to_write
	;
	char buf[255]={0};
	;

	if(0==on_off) { // ZERO=REMOVE CHEAT.  1=INSTALL CHEAT
		memory_contents=nop_code;
		cheat_contents=original_code;
	}

	lp_game_memory_address= g_baseAddress+p_definition->relative_offset;

	hProcess = OpenProcess
	(	STANDARD_RIGHTS_REQUIRED | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE
	,	FALSE
	,	g_process_id
	);
	if(NULL==hProcess)
		return file_log("Error in OpenProcess")?FALSE:FALSE;
	b_res=ReadProcessMemory
	(	hProcess // [in]  HANDLE  hProcess
	,	lp_game_memory_address // [in]  LPCVOID lpBaseAddress
	,	check_buffer // [out] LPVOID  lpBuffer
	,	p_definition->cheat_num_bytes // [in]  SIZE_T  nSize
	,	&NumberOfBytesRead // [out] SIZE_T  *lpNumberOfBytesRead
	);
	if(0 == b_res) {
		CloseHandle(hProcess);
		return file_log("Error reading memory")?FALSE:FALSE;
	}

	if(NumberOfBytesRead != p_definition->cheat_num_bytes) {
		CloseHandle(hProcess);
		return file_log("Size mismatch reading memory")?FALSE:FALSE;
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
		return file_log("Error writing memory")?FALSE:FALSE;
	}
	if(NumberOfBytesWritten != p_definition->cheat_num_bytes) {
		CloseHandle(hProcess);
		return file_log("Size mismatch reading memory")?FALSE:FALSE;
	}
	CloseHandle(hProcess);
	Sleep(1000);
	return 1;
}



int wait_for_process_and_inject()
{
	int n_process_id=0;

	do {
		n_process_id=find_process_id();
		if(0!=n_process_id)
			break;
		file_log("L %d: waiting for '%s'", __LINE__, TARGET_EXE);
		Sleep(500);
	} while(0==n_process_id);
	g_process_id=n_process_id;
	return perform_dll_injection();
}


