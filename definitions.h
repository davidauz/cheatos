#ifndef DEFS_H_
#define DEFS_H_

enum cheats
{	CHEAT_AMMO=0
,	CHEAT_LIFE
};

static struct cheat_definition {
	TCHAR	*cheat_prompt;
	char	*original_code;
	char	*cheat_code;
	int	memory_address;
	int	cheat_num_bytes;
}  definitions[] =
{	[CHEAT_AMMO]=
	{	L"Infinite ammo"
	,	"\x89\x4f\x30" // mov     dword ptr [rdi+30h],ecx (move the amount of available bullets in this place)
	,	"\x90\x90\x90" // nop nop nop
	,	0x66cf5b
	,	3
	}
,	[CHEAT_LIFE]=
	{	L"Infinite life"
	,	"\x2b\xd3" // sub     edx,ebx (subctract damage (ebx) from life value (edx))
	,	"\x90\x90" // nop nop
	,	0x90fdb2
	,	2
	}	
};

#endif
