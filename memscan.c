
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <tlhelp32.h>
#include "definitions.h"
#include "logic.h"

extern struct cheat_definition definitions[];

BOOL IsReadableMemory(LPVOID address) {
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(address, &mbi, sizeof(mbi)) == 0)
		return FALSE;
	
	return (mbi.State == MEM_COMMIT && 
		!(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)));
}

void ScanMemoryRange
(	LPVOID start_addr
,	SIZE_T size
,	BYTE *sequence
,	int sequence_length
){
    if (!start_addr || !sequence || sequence_length <= 0 || size < (SIZE_T)sequence_length)
        return;

    BYTE *base = (BYTE *)start_addr;
    SIZE_T max = size - sequence_length;

    for (SIZE_T i = 0; i <= max; ++i) {
        if (base[i] == sequence[0]) {
            if (memcmp(base + i, sequence, sequence_length) == 0) {
		file_log("%s:%d FOUND at 0x%p", __FILE__, __LINE__, base+i);
		return;
            }
        }
    }
}

void ScanCurrentProcessMemory() {
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		file_log("%s:%d Error: CreateToolhelp32Snapshot failed. Error: %lu\n", __FILE__, __LINE__, GetLastError());
		return;
	}
	MODULEENTRY32 me32 = {0};
	BYTE* calculated_end;
	me32.dwSize = sizeof(MODULEENTRY32);
	if (Module32First(hSnapshot, &me32)) {
		do {
			if(!strcmp(TARGET_EXE, me32.szModule)){
				file_log("%s:%d Module: %-30s", __FILE__, __LINE__, me32.szModule);
				file_log("%s:%d Base: 0x%p", __FILE__, __LINE__, me32.modBaseAddr);
				file_log("%s:%d size: 0x%08X bytes", __FILE__, __LINE__, me32.modBaseSize);
				calculated_end= (BYTE*)me32.modBaseAddr + me32.modBaseSize - 1;
				file_log("%s:%d end: 0x%p ", __FILE__, __LINE__, calculated_end);
				file_log("%s:%d Path: %s ", __FILE__, __LINE__, me32.szExePath);
				ScanMemoryRange
				(	me32.modBaseAddr
				,	me32.modBaseSize
				,	definitions[LETS_TICK].original_code
				,	definitions[LETS_TICK].cheat_num_bytes
				);
				file_log("%s:%d 1", __FILE__, __LINE__);
			}
		} while (Module32Next(hSnapshot, &me32));
	}
	file_log("%s:%d 0", __FILE__, __LINE__);
}

