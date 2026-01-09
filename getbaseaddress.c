

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <tlhelp32.h>
#include "logic.h"
#include "getbaseaddress.h"

BYTE *modBaseAddr=0x0;
SIZE_T modBaseSize=0;

int query_module_parameters() {
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return file_log("%s : %d Error: CreateToolhelp32Snapshot failed. Error: %lu\n", __FILE__, __LINE__, GetLastError());

	MODULEENTRY32 me32 = {0};
	me32.dwSize = sizeof(MODULEENTRY32);
	if (Module32First(hSnapshot, &me32)) {
		do {
			if(!strcmp(TARGET_EXE, me32.szModule)){
				file_log("%s : %d Module: %-30s", __FILE__, __LINE__, me32.szModule);
				modBaseAddr = me32.modBaseAddr;
				file_log("%s : %d Base: 0x%p", __FILE__, __LINE__, me32.modBaseAddr);
				file_log("%s : %d size: 0x%08X bytes", __FILE__, __LINE__, me32.modBaseSize);
				modBaseSize = me32.modBaseSize;
				file_log("%s : %d Path: %s ", __FILE__, __LINE__, me32.szExePath);
				return 0;
			}
		} while (Module32Next(hSnapshot, &me32));
	}
	file_log("%s : %d Target module not found", __FILE__, __LINE__);
	return 255;
}

BYTE *getBaseAddress(){
	if(0x0==modBaseAddr)
		query_module_parameters();
	return modBaseAddr;
}

SIZE_T getBaseSize(){
	if(0==modBaseSize)
		query_module_parameters();
	return modBaseSize;
}

