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
	,	strlen(buf)	// number of bytes to write
	,	&dwBytesWritten	// number of bytes that were written
	,	NULL      // no overlapped structure
	);
	CloseHandle(report_file_handle);
	return 0;
}


long int g_integer;
float g_float;

void reset_acceleration_value(){
	g_float=1;
}

void increase_acceleration_value(){
	g_float+=5;
	file_log( "%s:%d now `%f`", __FILE__, __LINE__, g_float );
}

// cheatos!speed_multiplier
void continuous_skidding(){ // actually useless
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
	"movss		0x18(%%rbx),%%xmm7;" // original "movss xmm7,dword ptr [rbx+18h]"
	"mov		0x45a8(%%rdi),%%rcx;" // original "mov rcx,qword ptr [rdi+45A8h]"
	"movl		%0, %%eax;"
	"addq		%%rax,%%rcx;"
	"movq		$1, %%rax;" // at this point it is always 1
	"ret;"
	:
	: "r" (g_integer)
	);
// N.B. at the beginning of the C function there is the function header:
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
to see the opcodes in speed_multiplier:
(gdb) disass /r speed_multiplier
*/
}

void due_volte_moltiplicazione(){
// this is the original code in the game at the target address GenerationZero_F+0x5e0e66:
//
//0x5e0e66 f3 0f 59 e3     	MULSS      XMM4,XMM3   <-- HERE (4 bytes)
//0x5e0e6a f3 0f 10 54 24 78       MOVSS      XMM2,dword ptr [RSP + local_290] (6 bytes)
//0x5e0e70 f3 0f 59 d3     MULSS      XMM2,XMM3 (4 bytes) (TOT 14 bytes)
//
//andando a vedere con gdb:
//   0x7ff714100e66:      mulss  %xmm3,%xmm4
//   0x7ff714100e6a:      movss  0x78(%rsp),%xmm2
//   0x7ff714100e70:      mulss  %xmm3,%xmm2
//

// At this place there are 5+7=12 bytes that can be used to store the far call to the cheat code.
// The original flow will be diverted here:
__asm__( 
	"mulss  %xmm3,%xmm4;"
	"mulss  %xmm3,%xmm4;" // facendo due volte la moltiplicazione in realta rallenta
	"movss  0x78(%rsp),%xmm2;"
	"mulss  %xmm3,%xmm2;"
	"ret;"
	);
// N.B. at the beginning of the C function there is the function header:
// 55		push   %rbp
// 48 89 e5	mov    %rsp,%rbp
// this means that the __asm__ code up here sits at speed_multiplier()+4

// This is the code that will be written at the target address.
// It is here for reference only, total 13 bytes	
__asm__ (
	"movabs $0x1122334455667788,%rax;" //  48 b8 88 77 66 55 44 33 22 11   (10 bytes)
	"call *%rax;" // ff d0   (2 bytes)
	"nop;" // 90 (1 byte)
	"nop;" // 90 (1 byte)
);

/*
to see the opcodes in speed_multiplier:
(gdb) disass /r speed_multiplier
*/
}


void speed_multiplier(){
// this is the original code in the game at the target location:
//
//GenerationZero_F+0x5e0e09:
//00007ff6`86380e09 45 0f 57 c9        xorps   xmm9,xmm9 (4b)
//00007ff6`86380e0d f3 44 0f 11 4c 24 50  movss   dword ptr [rsp+50h],xmm9 (7b)
//00007ff6`86380e14 45 0f 57 d2        xorps   xmm10,xmm10 (4b) TOT 15b
//00007ff6`86380e18 f3440f11542454  movss   dword ptr [rsp+54h],xmm10
//00007ff6`86380e1f 450f57db        xorps   xmm11,xmm11
//00007ff6`86380e23 f3440f115c2458  movss   dword ptr [rsp+58h],xmm11
//00007ff6`86380e2a f3440f102d75d93101 movss xmm13,dword ptr [GenerationZero_F+0x18fe7a8 (00007ff6`8769e7a8)]
//00007ff6`86380e33 450f2fe7        comiss  xmm12,xmm15
//
//(gdb) x/i 0x7ff686380e09
//0x7ff686380e09:      xorps  %xmm9,%xmm9
//0x7ff686380e0d:      movss  %xmm9,0x50(%rsp)
//0x7ff686380e14:      xorps  %xmm10,%xmm10
//0x7ff686380e18:      movss  %xmm10,0x54(%rsp)
//0x7ff686380e1f:      xorps  %xmm11,%xmm11
//0x7ff686380e23:      movss  %xmm11,0x58(%rsp)
//0x7ff686380e2a:      movss  0x131d975(%rip),%xmm13        # 0x7ff68769e7a8
//0x7ff686380e33:      comiss %xmm15,%xmm12
//0x7ff686380e37:      jbe    0x7ff686380ed7

__asm__( 
	"movss	%0,%%xmm9;"
	"mulss	%%xmm12, %%xmm9;" // tanto viene azzerato subito
	"xorps  %%xmm9,%%xmm9;"
	"movss  %%xmm9,0x50(%%rsp);"
	"xorps  %%xmm10,%%xmm10;"
	"ret;"
	:
	: "m" (g_float)
	);
// N.B. at the beginning of the speed_multiplier C function there is the function header:
// 55		push   %rbp
// 48 89 e5	mov    %rsp,%rbp
// this means that the __asm__ code up here sits at speed_multiplier()+4

__asm__ (
	"movabs $0x1122334455667788,%rax;" //  48 b8 88 77 66 55 44 33 22 11   (10 bytes)
	"call *%rax;" // ff d0   (2 bytes)
	"nop;" // 90 (1 byte)
	"nop;" // 90 (1 byte)
	"nop;" // 90 (1 byte): TOT 15b
);

/*
to see the opcodes in speed_multiplier:
(gdb) disass /r speed_multiplier
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

 	p_new_definition->code_cave = p_definition->code_cave;
 	p_new_definition->relative_offset = p_definition->relative_offset;
 	p_new_definition->cheat_num_bytes = p_definition->cheat_num_bytes;

	p_where_to_write=(BYTE *)(p_new_definition->cheat_code);
	opcode=0x48; // 48B8 = MOVABS RAX, ...
	*p_where_to_write++=opcode;

	opcode=0xb8;
	*p_where_to_write++=opcode;

	codecave_address = (unsigned long long)p_codecave_function;
	codecave_address+=4;// to compensate for function header

	for(int idx=0; idx<8; idx++) {
		opcode=codecave_address & 0xFF;
		*p_where_to_write++=opcode;
		codecave_address=codecave_address >> 8;
	}

	opcode=0xff; // FFD0 = CALL RAX
	*p_where_to_write++=opcode;

	opcode=0xd0;
	*p_where_to_write++=opcode;

	opcode=0x90; // NOP
	*p_where_to_write++=opcode;
	*p_where_to_write++=opcode;
	*p_where_to_write++=opcode;

	return p_new_definition;
}



int perform_action(int cheat_id, bool on_off) {
	struct cheat_definition *p_definition=&definitions[cheat_id];

	if(0==g_process_id)
		find_process_id();
	if(0==g_process_id)
		return file_log("Error in find_process_id")?FALSE:FALSE;

	if( 0 != p_definition->code_cave)
		p_definition=do_codecave(p_definition);

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
// check the original contents only if not a codecave (because the address is calculatd each time)
	if(	0 == p_definition->code_cave // if NOT a codecave
	) {
		for(int i=0; i<p_definition->cheat_num_bytes; i++)
			if(memory_contents[i]!=check_buffer[i]) {
				CloseHandle(hProcess);
		file_log("L %d: 0x%X 0x%X 0x%X 0x%X 0x%X", __LINE__
		,	check_buffer[0]
		,	check_buffer[1]
		,	check_buffer[2]
		,	check_buffer[3]
		,	check_buffer[4]
		);
		file_log( "L %d: 0x%X 0x%X 0x%X 0x%X 0x%X", __LINE__
		,	memory_contents[0]
		,	memory_contents[1]
		,	memory_contents[2]
		,	memory_contents[3]
		,	memory_contents[4]
		);
				return file_log("L %d Original memory content don't match", __LINE__)?FALSE:FALSE;
		}
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
	reset_acceleration_value();

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


