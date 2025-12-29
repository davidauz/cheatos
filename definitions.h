#ifndef DEFS_H_
#define DEFS_H_

#define ERROR_VALUE	255

extern WCHAR g_log_file_name[MAX_PATH];

void prepare_shared_memory();

enum cheats
{	INFINITE_AMMO=0
,	INFINITE_LIFE	//	1
,	NO_RECHARGE	//	2
,	INFINITE_STAMINA//	3
,	LETS_KILL	//	4
,	ZERO_WEIGHT	//	5
,	LETS_FLY	//	6
,	LETS_RUN	//	7
,	LETS_TICK	//	8
,	LAST_ENTRY	//	9
};

void flying_codecave();
void super_speed_codecave();
void easy_kill_codecave();
void move_clock_codecave();

struct cheat_definition {
	char		*cheat_prompt;
	BYTE		original_code[0x20];
	BYTE		cheat_code[0x20];
	BYTE		*destination_address;
	int		cheat_num_bytes;
};

struct cheat_definition *get_definition(int );
void update_codecave_address (int def_seq,BYTE *actual_address_to_write);

#endif

