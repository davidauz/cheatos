
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <tlhelp32.h>
#include "definitions.h"
#include "logic.h"
#include "getbaseaddress.h"


void ScanMemoryRange
(	LPVOID mem_range_start_addr
,	SIZE_T mem_range_size
,	struct cheat_definition *def
){
	if (!mem_range_start_addr || !def )
		return;

	int sequence_length = def->aob_num_bytes;
	BYTE *base = (BYTE *)mem_range_start_addr
	,	*sequence=def->original_code;
	SIZE_T max = mem_range_size - sequence_length;
	file_log("%s : %d looking for '%s'", __FILE__, __LINE__, def->cheat_prompt);

	for (SIZE_T i = 0; i <= max; ++i) {
		if (base[i] == sequence[0]) {
			if (memcmp(base + i, sequence, sequence_length) == 0) {
				def->destination_address=base+i;
				file_log("%s : %d FOUND '%s' at 0x%p", __FILE__, __LINE__, def->cheat_prompt, def->destination_address);
				return;
			}
		}
	}
}

void ScanCurrentProcessMemory() {
	int index=INFINITE_AMMO; // 0
	while(true) {
		if(0==get_definition(index)->aob_num_bytes)
			return;
		ScanMemoryRange
		(	getBaseAddress()
		,	getBaseSize()
		,	get_definition(index)
		);
		index++;
	}
}

