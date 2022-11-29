#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <shlwapi.h>
#include <stdbool.h>

#include "logic.h"
#include "definitions.h"

#define UNINITIALIZED 0xFFFFFFFF
#define TARGET_EXE L"GenerationZero_F.exe"
//#define TARGET_EXE L"notepad.exe" // for tests

BYTE * g_baseAddress=0;
DWORD g_process_id=0;

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

__asm__(
	"push		%rax;"
	"subl		$16, %esp;" // simulated push xmm0
	"movdqu		%xmm0,(%esp) ;" // MOVDQU	Move Unaligned Double Quadword
	"movq		$3, %rax;" // 3 is the factor
	"movq		%rax, %xmm0;"
	"mulss		%xmm6, %xmm0;"
	"movss		18(%rbx), %xmm7;" // original instruction: movss xmm7,[rbx+18]
	"movdqu		(%esp), %xmm0;" // simulated pop xmm0
	"add		$16, %esp;"
	"pop		%rax;"
	"ret;"
	);
}

/*
esempio:
// is this enable or disable?  I didn't check the manual
void set_caching_x86(void) {
    long tmp;      // mov to/from cr requires a 64bit reg in 64bit mode
    asm volatile(
      "mov   %%cr0, %[tmp]\n\t"     // Note the double-% when we want a literal % in the asm output
      "or    $0x40000000, %[tmp]\n\t"
      "mov   %[tmp], %%cr0\n\t"
      "wbinvd\n\t"
      : [tmp] "=r" (tmp) // outputs
      : // no inputs
      : // no clobbers.  "memory" clobber isn't needed, this just affects performance, not contents
      );
}

example:
movl 4(%eax), %ebx
takes value inside register %eax, adds 4 to it, and then
fetches the contents of memory at that address, putting
the result into register %ebx; sometimes called a "load"
instruction as it loads data from memory into a register

*/



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




int find_process_id(){
	PROCESSENTRY32 pe32;
	HANDLE hProcess;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	g_baseAddress = 0;
	g_process_id = 0;
	if( hProcessSnap == INVALID_HANDLE_VALUE ){
		show_error_return_BS(L"Error in CreateToolhelp32Snapshot");
		return 0;
	}

	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( !Process32First( hProcessSnap, &pe32 ) ) {
		CloseHandle( hProcessSnap );
		show_error_return_BS (L"Error in Process32First");
		return 0;
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
		return show_error_return_false(L"Error in GetFullPathNameA");
	int	n_path_size=1+strlen(dll_path);
	HANDLE hProcess = OpenProcess
	(	STANDARD_RIGHTS_REQUIRED | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE
	,	FALSE
	,	g_process_id
	);
	if(NULL==hProcess)
		return show_error_return_false(L"Error in OpenProcess");
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
		return show_error_return_false(L"Error writing memory");
	}
	if(NumberOfBytesWritten != n_path_size) {
		CloseHandle(hProcess);
		return show_error_return_false(L"Size mismatch reading memory");
	}
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
//	if(0==b_res)
//		show_error_return_false(L"Error in VirtualFreeEx");

	CloseHandle(hProcess);

	return 0;
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

	if(0==on_off) { // ZERO=REMOVE CHEAT.  1=INSTALL CHEAT
		memory_contents=nop_code;
		cheat_contents=original_code;
	}

	if(0==g_process_id)
		find_process_id();
	if(0==g_process_id)
		return show_error_return_false(L"Error in find_process_id");

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



int wait_for_process_and_inject()
{
	int n_process_id=0;

	do {
		n_process_id=find_process_id();
		if(0!=n_process_id)
			break;
		Sleep(500);
	} while(0==n_process_id);
	g_process_id=n_process_id;
	return perform_dll_injection();
}


