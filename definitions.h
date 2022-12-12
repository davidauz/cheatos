#ifndef DEFS_H_
#define DEFS_H_

enum cheats
{	CHEAT_AMMO=0
,	CHEAT_LIFE
,	CHEAT_NO_RECHARGE
,	INFINITE_STAMINA
,	MOVEMENT_SPEED
};


void codecave();

static struct cheat_definition {
	char		*cheat_prompt;
	BYTE		*original_code;
	BYTE		*cheat_code;
	void		*code_cave;
	int		code_cave_length;
	long		relative_offset;
	int		cheat_num_bytes;
}  definitions[] =
{	[CHEAT_AMMO]=
	{	"Infinite ammo (numpad 1)"
	,	"\x89\x4f\x30" // mov     dword ptr [rdi+30h],ecx (move the amount of available bullets in this place)
	,	"\x90\x90\x90" // nop nop nop
	,	0x00 // code cave not used
	,	0
	,	0x66cf5b
	,	3
	}
,	[CHEAT_LIFE]=
	{	"Infinite life (numpad 2)"
	,	"\x2b\xd3" // sub     edx,ebx (subtract damage (ebx) from life value (edx))
	,	"\x90\x90" // nop nop
	,	0x00 // code cave not used
	,	0
	,	0x90fdb2
	,	2
	}	
,	[CHEAT_NO_RECHARGE]=
	{	"No recharge (numpad 3)"
	,	"\x89\x91\x7c\x02\x00\x00"
	,	"\x90\x90\x90\x90\x90\x90"
	,	0x00 // code cave not used
	,	0
	,	0x6b70f8
	,	6
	}
,	[INFINITE_STAMINA]=
	{	"Infinite stamina (numpad 4)"
	,	"\x74\x4d" // je      GenerationZero_F+0x5b777f
	,	"\x90\x90"
	,	0x00 // code cave not used
	,	0
	,	0x5b7730
	,	2
	}
,	[MOVEMENT_SPEED]=
	{	"Movement Speed (numpad 5;+6,-7)" // cheat_prompt
	,	"\xF3\x0F\x10\x7B\x18" // movss xmm7,[rbx+18]
	,	"\xE8\x00\x00\x00\x00" // CALL 00000000
	,	"\x50"	//	push   %rax
		"\x83\xec\x10"	//	sub    $0x10,%esp
		"\x67\xf3\x0f\x7f\x04\x24"	//	movdqu %xmm0,(%esp)
		"\x48\xc7\xc0\x03\x00\x00\x00"	//	mov    $0x3,%rax
		"\x66\x48\x0f\x6e\xc0"	//	movq   %rax,%xmm0
		"\xf3\x0f\x59\xc6"	//	mulss  %xmm6,%xmm0
		"\xf3\x0f\x10\x7b\x18"	//	movss  0x18(%rbx),%xmm7
		"\x67\xf3\x0f\x6f\x04\x24"	//	movdqu (%esp),%xmm0
		"\x83\xc4\x10"	//	add    $0x10,%esp
		"\x58"	//	pop    %rax
		"\xc3"	//	ret
		"\x90"	//	nop
		"\x5d"	//	pop    %rbp
		"\xc3"	//	ret
	,	42
	,	0x5e0da3// 0x190F is test_mem, g0 is at 0x5e0da3 // relative_offset
	,	5
	}
};

#endif
