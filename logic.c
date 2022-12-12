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
//#define TARGET_EXE "test_mem_analysis.exe" // for tests

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



void codecave(){
//https://community.intel.com/t5/Intel-C-Compiler/Jumping-to-Labels-in-Inline-Assembly/td-p/965904?attachment-id=50160
//https://gcc.gnu.org/onlinedocs/gcc-4.9.4/gnat_ugn_unw/A-Simple-Example-of-Inline-Assembler.html
//https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
//https://stackoverflow.com/questions/21245245/c-uses-assemble-operand-type-mismatch-for-push
//https://stackoverflow.com/questions/5397677/how-to-set-a-variable-in-gcc-with-intel-syntax-inline-assembly
//https://stackoverflow.com/questions/67241134/why-does-this-inline-assembly-code-not-work-in-my-c-program
//https://wiki.osdev.org/Inline_Assembly/Examples
//https://www.codeproject.com/Articles/15971/Using-Inline-Assembly-in-C-C
//https://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html#s3
//http://gec.di.uminho.pt/Discip/IA32_gas/Linux-InlineAssembly.pdf

// this is only for compiling the assembler.
// it is never actually used
__asm__( 
	"push		%rax;"
	"subl		$16, %esp;" // simulated push xmm0
	"movdqu		%xmm0,(%esp) ;" // MOVDQU	Move Unaligned Double Quadword
	"movq		$3, %rax;" // 3 is the factor
	"movq		%rax, %xmm0;"
	"mulss		%xmm6, %xmm0;"
	"movss		0x18(%rbx), %xmm7;" // original instruction: movss xmm7,[rbx+18]
	"movdqu		(%esp), %xmm0;" // simulated pop xmm0
	"add		$16, %esp;"
	"pop		%rax;"
	"ret;"
	);
/*
once compiled the opcodes can be seen in gdb:
(gdb) disass /r codecave
Dump of assembler code for function codecave:
   0x00000001400015d3 <+0>:     55      push   %rbp
   0x00000001400015d4 <+1>:     48 89 e5        mov    %rsp,%rbp
   0x00000001400015d7 <+4>:     50      push   %rax
   0x00000001400015d8 <+5>:     83 ec 10        sub    $0x10,%esp
   0x00000001400015db <+8>:     67 f3 0f 7f 04 24       movdqu %xmm0,(%esp)
   0x00000001400015e1 <+14>:    48 c7 c0 03 00 00 00    mov    $0x3,%rax
   0x00000001400015e8 <+21>:    66 48 0f 6e c0  movq   %rax,%xmm0
   0x00000001400015ed <+26>:    f3 0f 59 c6     mulss  %xmm6,%xmm0
   0x00000001400015f1 <+30>:    f3 0f 10 7b 18  movss  0x18(%rbx),%xmm7
   0x00000001400015f6 <+35>:    67 f3 0f 6f 04 24       movdqu (%esp),%xmm0
   0x00000001400015fc <+41>:    83 c4 10        add    $0x10,%esp
   0x00000001400015ff <+44>:    58      pop    %rax
   0x0000000140001600 <+45>:    c3      ret
   0x0000000140001601 <+46>:    90      nop
   0x0000000140001602 <+47>:    5d      pop    %rbp
   0x0000000140001603 <+48>:    c3      ret
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
// this is needed for writing at the original code location
	DWORD		jump_offset
	;
	BYTE		opcode
	,		*p_where_to_write
	;
	unsigned long	codecave_relative_offset
	;
	char buf[255]={0};
	;
	struct cheat_definition *p_new_definition=malloc(sizeof(struct cheat_definition));

	p_new_definition->cheat_prompt=malloc(1+strlen(p_definition->cheat_prompt));
	strcpy(p_new_definition->cheat_prompt, p_definition->cheat_prompt);

	p_new_definition->original_code=malloc(1+p_definition->cheat_num_bytes);
 	memcpy(p_new_definition->original_code, p_definition->original_code, 1+p_definition->cheat_num_bytes);

	p_new_definition->cheat_code=malloc(1+strlen(p_definition->cheat_code));
	p_new_definition->cheat_code[0]='\xE8';

	p_new_definition->code_cave=malloc(1+strlen(p_definition->code_cave));
 	memcpy(p_new_definition->code_cave, p_definition->code_cave, p_definition->code_cave_length);

 	p_new_definition->code_cave_length = p_definition->code_cave_length;
 	p_new_definition->relative_offset = p_definition->relative_offset;
 	p_new_definition->cheat_num_bytes = p_definition->cheat_num_bytes;

	file_log("L %d: base address='0x%.16llX'", __LINE__, g_baseAddress);
	codecave_relative_offset=p_new_definition->code_cave-(void *)g_baseAddress;
	file_log("L %d: codecave offset relative to base address='0x%X'", __LINE__, codecave_relative_offset); // 199fa190
	file_log("L %d: cheat target relative to base address='0x%X'", __LINE__, p_new_definition->relative_offset);
	if( codecave_relative_offset > p_new_definition->relative_offset ) {
// forward relative jump
		jump_offset= codecave_relative_offset - p_new_definition->relative_offset ;
	} else {
// backwards relative jump
		jump_offset= p_new_definition->relative_offset - codecave_relative_offset ;
		jump_offset=0xffffffff-jump_offset;
	}
	file_log("L %d: jump offset='0x%X'", __LINE__, jump_offset);
	p_where_to_write=(BYTE *)(p_new_definition->cheat_code);
	opcode=0xE8;
	file_log("L %d: jump_offset=`0x%x`,opcode:'0x%x'", __LINE__ , jump_offset, opcode);
	*p_where_to_write++=opcode;
	file_log("L %d: bytes to write: '0x%x 0x%x 0x%x 0x%x 0x%x '", __LINE__ , p_new_definition->cheat_code[0] , p_new_definition->cheat_code[1] , p_new_definition->cheat_code[2] , p_new_definition->cheat_code[3] , p_new_definition->cheat_code[4]);

	opcode=jump_offset & 0xFF;
	file_log("L %d: jump_offset=`0x%x`,opcode:'0x%x'", __LINE__ , jump_offset, opcode);
	*p_where_to_write++=opcode;
	file_log("L %d: bytes to write: '0x%x 0x%x 0x%x 0x%x 0x%x '", __LINE__ , p_new_definition->cheat_code[0] , p_new_definition->cheat_code[1] , p_new_definition->cheat_code[2] , p_new_definition->cheat_code[3] , p_new_definition->cheat_code[4]);

	jump_offset=jump_offset >> 8;
	opcode=jump_offset & 0xFF;
	file_log("L %d: jump_offset=`0x%x`,opcode:'0x%x'", __LINE__ , jump_offset, opcode);
	*p_where_to_write++=opcode;
	file_log("L %d: bytes to write: '0x%x 0x%x 0x%x 0x%x 0x%x '", __LINE__ , p_new_definition->cheat_code[0] , p_new_definition->cheat_code[1] , p_new_definition->cheat_code[2] , p_new_definition->cheat_code[3] , p_new_definition->cheat_code[4]);

	jump_offset=jump_offset >> 8;
	opcode=jump_offset & 0xFF;
	file_log("L %d: jump_offset=`0x%x`,opcode:'0x%x'", __LINE__ , jump_offset, opcode);
	*p_where_to_write++=opcode;
	file_log("L %d: bytes to write: '0x%x 0x%x 0x%x 0x%x 0x%x '", __LINE__ , p_new_definition->cheat_code[0] , p_new_definition->cheat_code[1] , p_new_definition->cheat_code[2] , p_new_definition->cheat_code[3] , p_new_definition->cheat_code[4]);

	jump_offset=jump_offset >> 8;
	opcode=jump_offset & 0xFF;
	file_log("L %d: jump_offset=`0x%x`,opcode:'0x%x'", __LINE__ , jump_offset, opcode);
	*p_where_to_write++=opcode;
	file_log("L %d: bytes to write: '0x%x 0x%x 0x%x 0x%x 0x%x '", __LINE__ , p_new_definition->cheat_code[0] , p_new_definition->cheat_code[1] , p_new_definition->cheat_code[2] , p_new_definition->cheat_code[3] , p_new_definition->cheat_code[4]);

	return p_new_definition;

//	* /* (unsigned char *) */ p_where_to_write= 255; // jump_offset;
//MessageBoxA(NULL,"A","INFO",MB_OK);
	/*
CALL CODECAVE
00007ff7`672a0daf e8fcfbc2ff      call    GenerationZero_F+0x2109b0 (00007ff7`66ed09b0)
CALLING ADDRESS 7ff7672a0daf=GenerationZero_F+0x5e0daf
NEXT INSTRCTION 7ff7672a0db4=GenerationZero_F+0x5e0db4
DESTION ADDRESS 7ff766ed09b0=GenerationZero_F+0x2109b0

5e0db4-2109b0=3D0404
672a0db4-66ed09b0=3D0404
ffffffff-3D0404=FFC2FBFB
*/
}


int perform_action(int cheat_id, bool on_off) {
	struct cheat_definition *p_definition=&definitions[cheat_id];

	if( 0 != p_definition->code_cave)
		p_definition=do_codecave(p_definition);
file_log("L %d: id=`%d`, on_off=`%d`", __LINE__, cheat_id, on_off);

	unsigned char	*nop_code = p_definition->cheat_code
	,	*original_code = p_definition->original_code
	,	check_buffer[]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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

	if(0==g_process_id)
		find_process_id();
	if(0==g_process_id)
		return file_log("Error in find_process_id")?FALSE:FALSE;
file_log("L %d", __LINE__);


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
	for(int i=0; i<p_definition->cheat_num_bytes; i++)
		if(memory_contents[i]!=check_buffer[i]) {
file_log("L %d", __LINE__);
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
			return file_log("Original memory content does not match")?FALSE:FALSE;
		}
file_log("L %d writing 0x%X 0x%X 0x%X 0x%X 0x%X ", __LINE__
,	cheat_contents[0]
,	cheat_contents[1]
,	cheat_contents[2]
,	cheat_contents[3]
,	cheat_contents[4]
);
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
