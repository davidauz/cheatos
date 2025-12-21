#ifndef DEFS_H_
#define DEFS_H_

#define ERROR_VALUE      255

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
};

void flying_codecave();
void super_speed_codecave();
void easy_kill_codecave();
void move_clock_codecave();

static struct cheat_definition {
	char		*cheat_prompt;
	BYTE		*original_code;
	BYTE		*cheat_code;
	long		relative_offset;
	int		cheat_num_bytes;
}  definitions[] =
{	[INFINITE_AMMO]=
	{	"Infinite ammo (numpad 1)"
	,	"\x89\x4f\x30" // mov     dword ptr [rdi+30h],ecx (move the amount of available bullets in this place)
	,	"\x90\x90\x90" // nop nop nop
	,	0x66cf5b
	,	3
	}
,	[INFINITE_LIFE]=
	{	"Infinite life (numpad 2)"
	,	"\x2b\xd3" // sub     edx,ebx (subtract damage (ebx) from life value (edx))
	,	"\x90\x90" // nop nop
	,	0x90fdb2
	,	2
	}	
,	[NO_RECHARGE]=
	{	"" // goes together with infinite ammo
	,	"\x89\x91\x7c\x02\x00\x00"
	,	"\x90\x90\x90\x90\x90\x90"
	,	0x6b70f8
	,	6
	}
,	[INFINITE_STAMINA]=
	{	"Infinite stamina (numpad 3)"
	,	"\x74\x4d" // je      GenerationZero_F+0x5b777f
	,	"\x90\x90"
	,	0x5b7730
	,	2
	}
,	[LETS_KILL]=
	{	"Easy kill (numpad 4)"
	,	"\xf3\x0f\x59\x53\x20"	//	mulss   xmm2,dword ptr [rbx+20h] (5 bytes)
		"\xf3\x0f\x59\xd0"	//	mulss   xmm2,xmm0 (4 bytes, tot 9 bytes)
	,	"\xFF\x15\xA6\x96\x9A\xFF" // call qword ptr [GenerationZero_F+0x460] (6 bytes) (0x450+0x10)
		"\x90\x90\x90"		//	nop nop nop (3 bytes)
	,	0x656DB4 // GenerationZero_F+0x656DB4
	,	9
	}
//  p /x 0XFFFFFFFF-(0x656DB4-0x450+5-0x10) =  0xff9a96a6
//           |            |      |  |   |
//           |            |      |  |   +- offset in jump table
//           |            |      |  +- next instruction is 5 bytes down
//           |            |      +- start of jump table
//           |            +- target relative offset
//           +- for negative value
,	[ZERO_WEIGHT]=
	{	"Zero Weight (numpad 5)"
	,	"\x0F\x57\xC0\xF3\x48\x0F\x2A\xC3" // xorps   xmm0,xmm0 - cvtsi2ss xmm0,rbx
	,	"\x0F\x57\xC0\x90\x90\x90\x90\x90" // xorps   xmm0,xmm0, nop
	,	0x65f5ec
	,	8
	}
,	[LETS_FLY]=
	{	"Flying (numpad 6)"
	,	"\x0F\x29\x7F\x40" // movaps  xmmword ptr [rdi+40h],xmm7 (movaps %xmm7,0x40(%rdi), 4 bytes)
		"\x0F\x29\x77\x50" // movaps  xmmword ptr [rdi+50h],xmm6 (movaps %xmm6,0x50(%rdi), 4 bytes)
	,	"\xFF\x15\x55\x0B\x84\xFE" // call    qword ptr [GenerationZero_F+0x450] ( call   *-0xFE840b55(%rip), 6 bytes)
		"\x90\x90" // nop nop (and even two bytes to spare)
// Here RIP is GenerationZero_F+0x17BF8F5
// Jump table is at GenerationZero_F+0x450; it is filled by init_jump_table() in dll_injection.c
// Jump table address relative to RIP is 0x17BF8F5-0x450+5 = 0x17BF4AA (+5 because RIP points to next instruction)
// But it has to be negative: 0XFFFFFFFF-0x17BF4AA = 0xFE840B55
// So up here is \xFF\x15 (call qword ptr) \x55\x0B\x84\xFE (address location)
// in short: p /x 0XFFFFFFFF-(0x17BF8F5-0x450+5) = 0xFE840B55
	,	0x17BF8F5
	,	8 // total 8 bytes
	}
,	[LETS_RUN]=
	{	"Running (numpad 7)"
	,	"\xf3\x0f\x10\x76\x1c"	// movss  0x1c(%rsi),%xmm6 (5 bytes)
		"\xf3\x0f\x5c\x76\x18"	// subss  0x18(%rsi),%xmm6 (5 bytes)
		"\xf3\x41\x0f\x59\xf6"	//  mulss  %xmm14,%xmm6 (5 bytes, tot 15 bytes)
	,	"\xFF\x15\x3a\xf7\xa1\xff" // call    qword ptr [GenerationZero_F+0x458] ( call   *-0xFFA1F73A(%rip) , 6 bytes)
		"\x90\x90\x90\x90" // nop nop nop nop (4 bytes)
		"\x90\x90\x90\x90\x90" // nop nop nop nop nop (5 bytes. total 15)
// p /x 0XFFFFFFFF-(0x5E0D18-0x450+5-8) = 0xFFA1F73A
	,	0x5e0d18 // GenerationZero_F+0x5e0d18
	,	15
	}
,	[LETS_TICK]=
	{	"Move clock (Numpad 8)"
	,	"\x0F\x2F\xC1"				// comiss %xmm1,%xmm0 (3 bytes) (compare xmm0 to xmm1)
		"\xF3\x0F\x11\x81\xE0\x00\x00\x00"	// movss  %xmm0,0xe0(%rcx) (8 bytes)
							// move scalar single-precision floating-point
							// Source: Lower 32 bits of XMM0 register
							// Destination: Memory at address RCX + 0xE0 (224 decimal offset)		
	,	"\xFF\x15\xfe\xbe\xc0\xff"		// call qword ptr [GenerationZero_F+0x468] (6 bytes)
		"\x90\x90\x90\x90\x90"			// nop nop nop nop nop (5 bytes)
							//(gdb) p /x0xFFFFFFFF-( 0x3F4564-0x450+5-0x18 )
							//= 0xffc0befe
	,	0x3F4564				// GenerationZero_F+0x3f4564
	,	11					// tot 11 bytes
	}
};

#endif

