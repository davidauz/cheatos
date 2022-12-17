#ifndef DEFS_H_
#define DEFS_H_

enum cheats
{	CHEAT_AMMO=0
,	CHEAT_LIFE
,	CHEAT_NO_RECHARGE
,	INFINITE_STAMINA
,	MOVEMENT_SPEED
};


void speed_multiplier();

static struct cheat_definition {
	char		*cheat_prompt;
	BYTE		*original_code;
	BYTE		*cheat_code;
	void		(*code_cave)();
	long		relative_offset;
	int		cheat_num_bytes;
}  definitions[] =
{	[CHEAT_AMMO]=
	{	"Infinite ammo (numpad 1)"
	,	"\x89\x4f\x30" // mov     dword ptr [rdi+30h],ecx (move the amount of available bullets in this place)
	,	"\x90\x90\x90" // nop nop nop
	,	0x00 // code cave not used
	,	0x66cf5b
	,	3
	}
,	[CHEAT_LIFE]=
	{	"Infinite life (numpad 2)"
	,	"\x2b\xd3" // sub     edx,ebx (subtract damage (ebx) from life value (edx))
	,	"\x90\x90" // nop nop
	,	0x00 // code cave not used
	,	0x90fdb2
	,	2
	}	
,	[CHEAT_NO_RECHARGE]=
	{	"No recharge (numpad 3)"
	,	"\x89\x91\x7c\x02\x00\x00"
	,	"\x90\x90\x90\x90\x90\x90"
	,	0x00 // code cave not used
	,	0x6b70f8
	,	6
	}
,	[INFINITE_STAMINA]=
	{	"Infinite stamina (numpad 4)"
	,	"\x74\x4d" // je      GenerationZero_F+0x5b777f
	,	"\x90\x90"
	,	0x00 // code cave not used
	,	0x5b7730
	,	2
	}
,	[MOVEMENT_SPEED]=
	{	"Movement Speed (numpad 5;+6,-7)" // cheat_prompt
	,	"\xf3\x0f\x59\xb0\xac\x00\x00\x00" // mulss   xmm6,dword ptr [rax+0ACh] (8 bytes)
		"\xf3\x0f\x10\x7b\x18" // movss   xmm7,dword ptr [rbx+18h] (5 bytes)
	,	"\x48\xa1\x88\x77\x66\x55\x44\x33\x22\x11" // movabs 0x1122334455667788,%rax; address will be calculated new each time
		"\xff\xd0" // call *%rax;
		"\x90" // nop;
	,	speed_multiplier
	,	0x5e0da3 // 0x1900 is test_mem, g0 is at GenerationZero_F+0x5e0da3
// bogus function at test_mem_analysis.exe+0x0000000000001900
	,	12
	}
};

#endif
