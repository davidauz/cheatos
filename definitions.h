#ifndef DEFS_H_
#define DEFS_H_

enum cheats
{	CHEAT_AMMO=0
,	CHEAT_LIFE
,	CHEAT_NO_RECHARGE
,	DLL_INJECTION
};

static struct cheat_definition {
	TCHAR	*cheat_prompt;
	char	*original_code;
	char	*cheat_code;
	int	memory_address;
	int	cheat_num_bytes;
}  definitions[] =
{	[CHEAT_AMMO]=
	{	L"Infinite ammo (numpad 1)"
	,	"\x89\x4f\x30" // mov     dword ptr [rdi+30h],ecx (move the amount of available bullets in this place)
	,	"\x90\x90\x90" // nop nop nop
	,	0x66cf5b
	,	3
	}
,	[CHEAT_LIFE]=
	{	L"Infinite life (numpad 2)"
	,	"\x2b\xd3" // sub     edx,ebx (subctract damage (ebx) from life value (edx))
	,	"\x90\x90" // nop nop
	,	0x90fdb2
	,	2
	}	
,	[CHEAT_NO_RECHARGE]=
	{	L"No recharge (numpad 3)"
	,	"\x89\x91\x7c\x02\x00\x00"
	,	"\x90\x90\x90\x90\x90\x90"
	,	0x6b70f8
	,	6
	}
,	[DLL_INJECTION]=
	{	L"Enable numpad"
	,	"cheatos.dll"
	,	""
	,	0x0
	,	0
	}
};
#endif
