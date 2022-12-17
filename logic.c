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

#define UNINITIALIZED 0xFFFFFFFF
#define TARGET_EXE "GenerationZero_F.exe"
//#define TARGET_EXE "notepad.exe" // for tests
//#define TARGET_EXE "test_mem_analysis.exe" // for tests (again)

BYTE * g_baseAddress=0;
DWORD g_process_id=0;


int file_log(char* format, ...){
	char buf[255]
	,	*filename="log.txt"
	;
	DWORD	dwBytesWritten;
	va_list argptr;
	va_start(argptr, format);
	vsprintf(buf, format, argptr);
	strcat(buf, "\n");
	va_end(argptr);
	HANDLE report_file_handle;
	report_file_handle = CreateFile
	(	filename	// file to write
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
	,	1+strlen(buf)	// number of bytes to write
	,	&dwBytesWritten	// number of bytes that were written
	,	NULL      // no overlapped structure
	);
	CloseHandle(report_file_handle);
	return 0;
}



void speed_multiplier(){
// this is the original code in the game at the target address GenerationZero_F+0x5e0d9b :
//
// 00007ff7`62280d9b f3 0f 59 b0 ac 00 00 00 mulss   xmm6,dword ptr [rax+0ACh]
// 00007ff7`62280da3 f3 0f 10 7b 18      movss   xmm7,dword ptr [rbx+18h] (5 bytes) <-- injection here
// 00007ff7`62280da8 48 8b 8f a8 45 00 00  mov     rcx,qword ptr [rdi+45A8h] (7 bytes)
// 00007ff7`62280daf e8fcfbc2ff      call    GenerationZero_F+0x2109b0 (00007ff7`61eb09b0)
// 00007ff7`62280db4 0fb6f0          movzx   esi,al
//
// At this place there are 5+7=12 bytes that can be used to store the far call to the cheat code.
// The original flow will be diverted here:
__asm__( 
	"subq		$0x16, %rsp;" // simulated push xmm0
	"movdqu		%xmm0,(%rsp) ;" // save xmm0 (Move Unaligned Double Quadword)
	"movq		$3, %rax;" // 3 is the multiplying factor
	"movq		%rax, %xmm0;" // move 3 from rax to xmm0
	"movss		0x18(%rbx),%xmm7;" // original "movss xmm7,dword ptr [rbx+18h]"
	"movdqu		(%rsp), %xmm0;" // simulated pop xmm0
	"addq		$0x16, %rsp;" // restore stack
	"movq		$1, %rax;" // at this point it is always 1
	"mov		0x45a8(%rdi),%rcx;" // original "mov rcx,qword ptr [rdi+45A8h]"
	"inc %rcx;" // diversi di questi sortiscono effetti curiosi
	"ret;"
	);
// N.B. at the beginning of the speed_multiplier C function there is the function header:
// 55		push   %rbp
// 48 89 e5	mov    %rsp,%rbp
// this means that the __asm__ code up here sits at speed_multiplier()+4

// This is the code that will be written at the target address.
// It is here for reference only, total 13 bytes	
__asm__ (
	"movabs $0x1122334455667788,%rax;" //  48 b8 88 77 66 55 44 33 22 11   (10 bytes)
	"call *%rax;" // ff d0   (2 bytes)
);

/*
here are the opcodes in speed_multiplier:
(gdb) disass /r speed_multiplier
Dump of assembler code for function speed_multiplier:
   0x00000001400016c8 <+0>:     55      push   %rbp
   0x00000001400016c9 <+1>:     48 89 e5        mov    %rsp,%rbp
   0x00000001400016cc <+4>:     50      push   %rax
   0x00000001400016cd <+5>:     83 ec 10        sub    $0x10,%esp
   0x00000001400016d0 <+8>:     67 f3 0f 7f 04 24       movdqu %xmm0,(%esp)
   0x00000001400016d6 <+14>:    48 c7 c0 03 00 00 00    mov    $0x3,%rax
   0x00000001400016dd <+21>:    66 48 0f 6e c0  movq   %rax,%xmm0
   0x00000001400016e2 <+26>:    f3 0f 59 c6     mulss  %xmm6,%xmm0
   0x00000001400016e6 <+30>:    f3 0f 10 7b 18  movss  0x18(%rbx),%xmm7
   0x00000001400016eb <+35>:    67 f3 0f 6f 04 24       movdqu (%esp),%xmm0
   0x00000001400016f1 <+41>:    83 c4 10        add    $0x10,%esp
   0x00000001400016f4 <+44>:    58      pop    %rax
   0x00000001400016f5 <+45>:    f3 0f 59 b0 ac 00 00 00 mulss  0xac(%rax),%xmm6
   0x00000001400016fd <+53>:    f3 0f 10 7b 18  movss  0x18(%rbx),%xmm7
   0x0000000140001702 <+58>:    c3      ret
   0x0000000140001703 <+59>:    48 a1 88 77 66 55 44 33 22 11   movabs 0x1122334455667788,%rax
   0x000000014000170d <+69>:    ff d0   call   *%rax
   0x000000014000170f <+71>:    90      nop
   0x0000000140001710 <+72>:    90      nop
   0x0000000140001711 <+73>:    5d      pop    %rbp
   0x0000000140001712 <+74>:    c3      ret
End of assembler dump.
*/
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
	int	n_path_size=1+strlen(dll_path);
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
	,	n_path_size // [in]           SIZE_T dwSize,
	,	MEM_COMMIT|MEM_RESERVE // [in]           DWORD  flAllocationType,
	,	PAGE_READWRITE // [in]           DWORD  flProtect
	);
	b_res = WriteProcessMemory
	(	hProcess //  [in]  HANDLE  hProcess
	,	p_dll_memory // [in]  LPVOID  lpBaseAddress
	,	dll_path // [in]  LPCVOID lpBuffer
	,	n_path_size //[in]  SIZE_T  nSize
	,	&NumberOfBytesWritten // [out] SIZE_T *lpNumberOfBytesWritten
	);
	if(0==b_res) {
		CloseHandle(hProcess);
		return file_log("Error writing memory")?FALSE:FALSE;
	}
	if(NumberOfBytesWritten != n_path_size) {
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
	,	n_path_size // [in] SIZE_T dwSize,
	,	MEM_RELEASE // [in] DWORD  dwFreeType
	);

	CloseHandle(hProcess);
	file_log("L %d: return", __LINE__);

	return 0;
}



struct cheat_definition * do_codecave(struct cheat_definition *p_definition) {
	BYTE		opcode
	,		*p_where_to_write
	;
	unsigned	long long codecave_address;
	void		*p_codecave_function = p_definition->code_cave;
	struct cheat_definition *p_new_definition=malloc(sizeof(struct cheat_definition));

	p_new_definition->cheat_prompt=malloc(1+strlen(p_definition->cheat_prompt));
	strcpy(p_new_definition->cheat_prompt, p_definition->cheat_prompt);

	p_new_definition->original_code=malloc(p_definition->cheat_num_bytes);
 	memcpy(p_new_definition->original_code, p_definition->original_code, 1+p_definition->cheat_num_bytes);

	p_new_definition->cheat_code=malloc(p_definition->cheat_num_bytes);
	// this will be written in a moment

 	p_new_definition->relative_offset = p_definition->relative_offset;
 	p_new_definition->cheat_num_bytes = p_definition->cheat_num_bytes;

	file_log("L %d: base address='0x%.16llX'", __LINE__, g_baseAddress);
	p_where_to_write=(BYTE *)(p_new_definition->cheat_code);
	opcode=0x48;
	file_log("L %d: writing `0x%x` at `0x%.16llX`", __LINE__, opcode, p_where_to_write);
	*p_where_to_write++=opcode;

	opcode=0xb8;
	file_log("L %d: writing `0x%x` at `0x%.16llX`", __LINE__, opcode, p_where_to_write);
	*p_where_to_write++=opcode;

	codecave_address = (unsigned long long)p_codecave_function;
	codecave_address+=4;// to compensate for function header
	file_log("L %d: codecave code at `0x%.16llX`", __LINE__, codecave_address);

	for(int idx=0; idx<8; idx++) {
		opcode=codecave_address & 0xFF;
		file_log("L %d: writing `0x%x` at `0x%.16llX`", __LINE__, opcode, p_where_to_write);
		*p_where_to_write++=opcode;
		codecave_address=codecave_address >> 8;
	}

	opcode=0xff;
	file_log("L %d: writing `0x%x` at `0x%.16llX`", __LINE__, opcode, p_where_to_write);
	*p_where_to_write++=opcode;

	opcode=0xd0;
	file_log("L %d: writing `0x%x` at `0x%.16llX`", __LINE__, opcode, p_where_to_write);
	*p_where_to_write++=opcode;

	return p_new_definition;
}



int perform_action(int cheat_id, bool on_off) {
	struct cheat_definition *p_definition=&definitions[cheat_id];

	if(0==g_process_id)
		find_process_id();
	if(0==g_process_id)
		return file_log("Error in find_process_id")?FALSE:FALSE;
file_log("L %d, base_address=`0x%.16llX`", __LINE__, g_baseAddress);

	if( 0 != p_definition->code_cave)
		p_definition=do_codecave(p_definition);
file_log("L %d: id=`%d`, on_off=`%d`", __LINE__, cheat_id, on_off);

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
file_log("L %d", __LINE__);

	hProcess = OpenProcess
	(	STANDARD_RIGHTS_REQUIRED | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE
	,	FALSE
	,	g_process_id
	);
file_log("L %d", __LINE__);
	if(NULL==hProcess)
		return file_log("Error in OpenProcess")?FALSE:FALSE;
file_log("L %d", __LINE__);
	b_res=ReadProcessMemory
	(	hProcess // [in]  HANDLE  hProcess
	,	lp_game_memory_address // [in]  LPCVOID lpBaseAddress
	,	check_buffer // [out] LPVOID  lpBuffer
	,	p_definition->cheat_num_bytes // [in]  SIZE_T  nSize
	,	&NumberOfBytesRead // [out] SIZE_T  *lpNumberOfBytesRead
	);
file_log("L %d", __LINE__);
	if(0 == b_res) {
file_log("L %d", __LINE__);
		CloseHandle(hProcess);
		return file_log("Error reading memory")?FALSE:FALSE;
	}
file_log("L %d", __LINE__);

	if(NumberOfBytesRead != p_definition->cheat_num_bytes) {
file_log("L %d", __LINE__);
		CloseHandle(hProcess);
		return file_log("Size mismatch reading memory")?FALSE:FALSE;
	}
// check the original contents only if not a codecave (because the address is calculatd each time)
//	if( 0 == p_definition->code_cave)
//		for(int i=0; i<p_definition->cheat_num_bytes; i++)
//			if(memory_contents[i]!=check_buffer[i]) {
//file_log("L %d", __LINE__);
//				CloseHandle(hProcess);
//		file_log("L %d: 0x%X 0x%X 0x%X 0x%X 0x%X", __LINE__
//		,	check_buffer[0]
//		,	check_buffer[1]
//		,	check_buffer[2]
//		,	check_buffer[3]
//		,	check_buffer[4]
//		);
//		file_log( "L %d: 0x%X 0x%X 0x%X 0x%X 0x%X", __LINE__
//		,	memory_contents[0]
//		,	memory_contents[1]
//		,	memory_contents[2]
//		,	memory_contents[3]
//		,	memory_contents[4]
//		);
//				return file_log("L %d Original memory content don't match", __LINE__)?FALSE:FALSE;
//		}
file_log("L %d writing 0x%X 0x%X 0x%X 0x%X 0x%X  at `0x%.16llX`", __LINE__
,	cheat_contents[0]
,	cheat_contents[1]
,	cheat_contents[2]
,	cheat_contents[3]
,	cheat_contents[4]
,	lp_game_memory_address
); // cheatos!perform_action+0x589 (+ o -)
	b_res = WriteProcessMemory
	(	hProcess //  [in]  HANDLE  hProcess
	,	lp_game_memory_address // [in]  LPVOID  lpBaseAddress
	,	(LPCVOID)cheat_contents // [in]  LPCVOID lpBuffer
	,	p_definition->cheat_num_bytes //[in]  SIZE_T  nSize
	,	&NumberOfBytesWritten // [out] SIZE_T *lpNumberOfBytesWritten
	);
file_log("L %d", __LINE__);
	if(0==b_res) {
		CloseHandle(hProcess);
		return file_log("Error writing memory")?FALSE:FALSE;
	}
file_log("L %d", __LINE__);
	if(NumberOfBytesWritten != p_definition->cheat_num_bytes) {
		CloseHandle(hProcess);
		return file_log("Size mismatch reading memory")?FALSE:FALSE;
	}
file_log("L %d", __LINE__);
	CloseHandle(hProcess);
file_log("L %d", __LINE__);
	Sleep(1000);
	return 1;
}



int wait_for_process_and_inject()
{

	int n_process_id=0;
	file_log("L %d: wait_for_process_and_inject", __LINE__);

	do {
		n_process_id=find_process_id();
		if(0!=n_process_id)
			break;
		file_log("L %d: waiting", __LINE__);
		Sleep(500);
	} while(0==n_process_id);
	g_process_id=n_process_id;
	return perform_dll_injection();
}


int debug_tests(){
	struct cheat_definition *p_definition=&definitions[MOVEMENT_SPEED];
	do_codecave(p_definition);
	return 0;
}

