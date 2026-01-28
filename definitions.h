#ifndef DEFS_H_
#define DEFS_H_

#define ERROR_VALUE	255

extern WCHAR g_log_file_name[MAX_PATH];

void prepare_shared_memory();

enum cheats
{	INFINITE_AMMO=0
,	INFINITE_AMMO_2
,	INFINITE_LIFE
//,	NO_RECHARGE
,	INFINITE_STAMINA
,	LETS_KILL
,	ZERO_WEIGHT
,	LETS_FLY
,	LETS_RUN
,	LETS_TICK
,	STORAGE_BOX
,	LAST_ENTRY
};


struct cheat_definition {
	char		*cheat_prompt;
	BYTE		original_code[0x40];
	BYTE		cheat_code[0x40];
	BYTE		*destination_address;
	int		cheat_num_bytes;
	int		aob_num_bytes;
};

struct cheat_definition *get_definition(int );
void update_codecave_address (int def_seq,BYTE *actual_address_to_write);

#endif

