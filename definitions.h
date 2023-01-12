#ifndef DEFS_H_
#define DEFS_H_

enum cheats
{	INFINITE_AMMO=0
,	INFINITE_LIFE //	1
,	NO_RECHARGE //		2
,	INFINITE_STAMINA //	3
,	ONE_HIT_KILL //		4
,	ZERO_WEIGHT  //		5
//,	MOVEMENT_SPEED TODO
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
{	[INFINITE_AMMO]=
	{	"Infinite ammo (numpad 1)"
	,	"\x89\x4f\x30" // mov     dword ptr [rdi+30h],ecx (move the amount of available bullets in this place)
	,	"\x90\x90\x90" // nop nop nop
	,	0 // code cave not used
	,	0x66cf5b
	,	3
	}
,	[INFINITE_LIFE]=
	{	"Infinite life (numpad 2)"
	,	"\x2b\xd3" // sub     edx,ebx (subtract damage (ebx) from life value (edx))
	,	"\x90\x90" // nop nop
	,	0 // code cave not used
	,	0x90fdb2
	,	2
	}	
,	[NO_RECHARGE]=
	{	"" // goes together with infinite ammo
	,	"\x89\x91\x7c\x02\x00\x00"
	,	"\x90\x90\x90\x90\x90\x90"
	,	0 // code cave not used
	,	0x6b70f8
	,	6
	}
,	[INFINITE_STAMINA]=
	{	"Infinite stamina (numpad 3)"
	,	"\x74\x4d" // je      GenerationZero_F+0x5b777f
	,	"\x90\x90"
	,	0 // code cave not used
	,	0x5b7730
	,	2
	}
,	[ONE_HIT_KILL]=
	{	"One Hit Kill (numpad 4)"
	,	"\xF3\x0F\x10\x83\xDC\x00\x00\x00" // movss   xmm0,dword ptr [rbx+0DCh]
	,	"\xE9\xD2\xB1\xA8\xBF\x90\x90\x90" // jmp + nop
	,	0 // code cave not used
	,	0x574e59
	,	8
	}
,	[ZERO_WEIGHT]=
	{	"Zero Weight (numpad 5)"
	,	"\x0F\x57\xC0\xF3\x48\x0F\x2A\xC3" // xorps   xmm0,xmm0 - cvtsi2ss xmm0,rbx
	,	"\x0F\x57\xC0\x90\x90\x90\x90\x90" // xorps   xmm0,xmm0, nop
	,	0 // code cave not used
	,	0x65f5ec
	,	8
	}
//,	[MOVEMENT_SPEED]= TODO
//	{	"Movement Speed (numpad 5;+6,-7)" // cheat_prompt
//	,	"\x45\x0f\x57\xc9" // xorps   xmm9,xmm9 (4b)
//		"\xf3\x44\x0f\x11\x4c\x24\x50" // movss   dword ptr [rsp+50h],xmm9 (7b)
//		"\x45\x0f\x57\xd2" // xorps   xmm10,xmm10 (4b) TOT 15b
//	,	"\x48\xb8\x88\x77\x66\x55\x44\x33\x22\x11" // movabs 0x1122334455667788,%rax; will be calculated every time (10b)
//		"\xff\xd0" // call *%rax; (2b)
//		"\x90\x90\x90" // nop nop; (3b, tot 15b)
//	,	speed_multiplier
//	,	0x5e0e09
//// bogus function at test_mem_analysis.exe+0x0000000000001900
//	,	15
//	}
};

#endif
