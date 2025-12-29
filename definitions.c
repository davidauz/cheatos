#include <windows.h>
#include <stdint.h>
#include "definitions.h"
#include "logic.h"

struct cheat_definition definitions[] =
{	[INFINITE_AMMO]=
	{	"Infinite ammo (numpad 1)"
	,	{0x89, 0x4f, 0x30 } // mov     dword ptr [rdi+30h],ecx (move the amount of available bullets in this place)
	,	{ 0x90, 0x90, 0x90 } // nop nop nop
	,	NULL
	,	3
	}
,	[INFINITE_LIFE]=
	{	"Infinite life (numpad 2)"
	,	{ 0x2b, 0xd3 } // sub     edx,ebx (subtract damage (ebx) from life value (edx))
	,	{ 0x90, 0x90 } // nop nop
	,	NULL
	,	2
	}	
,	[NO_RECHARGE]=
	{	"" // goes together with infinite ammo
	,	{ 0x89, 0x91, 0x7c, 0x02, 0x00, 0x00 }
	,	{ 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }
	,	NULL
	,	6
	}
,	[INFINITE_STAMINA]=
	{	"Infinite stamina (numpad 3)"
	,	{ 0x74, 0x4d } // je      GenerationZero_F+0x5b777f
	,	{ 0x90, 0x90 }
	,	NULL
	,	2
	}
,	[LETS_KILL]=
	{	"Easy kill (numpad 4)"
	,	{ 0xf3, 0x0f, 0x59, 0x53, 0x20 	//	mulss   xmm2,dword ptr [rbx+20h] (5 bytes)
		, 0xf3, 0x0f, 0x59, 0xd0 }	//	mulss   xmm2,xmm0 (4 bytes, tot 9 bytes)
	,	{ 0xFF, 0x15, 0xA6, 0x96, 0x9A, 0xFF  // call qword ptr [GenerationZero_F+0x460] (6 bytes) (0x450+0x10)
		, 0x90, 0x90, 0x90 }		//	nop nop nop (3 bytes)
// \xFF \x15 stands for CALL QWORD PTR [RIP + displacement] or "call through memory pointer".
// What follows the two opcodes is a 32-bit little-endian displacement.
// The target address is calculated as Next RIP + displacement
// where 'Next RIP' is the address of the next instruction after the complete 6-byte call
// So suppose that the instruction starts at: 0x7ff96b304d3d: \xff \x15 11 22 33 44
// The instruction length is of : 6 bytes
// The Next RIP is at 0x7ff96b304d3d + 6 = 0x7ff96b304d43
	,	NULL
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
	,	{ 0x0F, 0x57, 0xC0, 0xF3, 0x48, 0x0F, 0x2A, 0xC3 } // xorps   xmm0,xmm0 - cvtsi2ss xmm0,rbx
	,	{ 0x0F, 0x57, 0xC0, 0x90, 0x90, 0x90, 0x90, 0x90 } // xorps   xmm0,xmm0, nop
	,	NULL
	,	8
	}
,	[LETS_FLY]=
	{	"Flying (numpad 6)"
	,	{ 0x0F, 0x29, 0x7F, 0x40  // movaps  xmmword ptr [rdi+40h],xmm7 (movaps %xmm7,0x40(%rdi), 4 bytes)
		, 0x0F, 0x29, 0x77, 0x50 } // movaps  xmmword ptr [rdi+50h],xmm6 (movaps %xmm6,0x50(%rdi), 4 bytes)
	,	{ 0xFF, 0x15, 0x55, 0x0B, 0x84, 0xFE  // call    qword ptr [GenerationZero_F+0x450] ( call   *-0xFE840b55(%rip), 6 bytes)
		, 0x90, 0x90 } // nop nop (and even two bytes to spare)
// Here RIP is GenerationZero_F+0x17BF8F5
// Jump table is at GenerationZero_F+0x450; it is filled by init_jump_table() in dll_injection.c
// Jump table address relative to RIP is 0x17BF8F5-0x450+5 = 0x17BF4AA (+5 because RIP points to next instruction)
// But it has to be negative: 0XFFFFFFFF-0x17BF4AA = 0xFE840B55
// So up here is \xFF\x15 (call qword ptr) \x55\x0B\x84\xFE (address location)
// in short: p /x 0XFFFFFFFF-(0x17BF8F5-0x450+5) = 0xFE840B55
	,	NULL
	,	8 // total 8 bytes
	}
,	[LETS_RUN]=
	{	"Running (numpad 7)"
	,	{ 0xf3, 0x0f, 0x10, 0x76, 0x1c 	// movss  0x1c(%rsi),%xmm6 (5 bytes)
		, 0xf3, 0x0f, 0x5c, 0x76, 0x18 	// subss  0x18(%rsi),%xmm6 (5 bytes)
		, 0xf3, 0x41, 0x0f, 0x59, 0xf6 }	//  mulss  %xmm14,%xmm6 (5 bytes, tot 15 bytes)
	,	{ 0xFF, 0x15, 0x3a, 0xf7, 0xa1, 0xff  // call    qword ptr [GenerationZero_F+0x458] ( call   *-0xFFA1F73A(%rip) , 6 bytes)
		, 0x90, 0x90, 0x90, 0x90  // nop nop nop nop (4 bytes)
		, 0x90, 0x90, 0x90, 0x90, 0x90 } // nop nop nop nop nop (5 bytes. total 15)
// p /x 0XFFFFFFFF-(0x5E0D18-0x450+5-8) = 0xFFA1F73A
	,	NULL
	,	15
	}
,	[LETS_TICK]=
	{	"Move clock (Numpad 8)"
	,	{ 0x0f, 0x28, 0xce  // movaps %xmm6,%xmm1
		, 0xf3, 0x0f, 0x59, 0x8b, 0xec, 0x00, 0x00, 0x00  //  mulss  0xec(%ebx),%xmm1
		, 0xf3, 0x0f, 0x58, 0xc8 } // addss  %xmm0,%xmm1
	,	{ 0xFF, 0x15, 0xfe, 0xbe, 0xc0, 0xff 		// call qword ptr [GenerationZero_F+0x468] (6 bytes)
		, 0x90, 0x90, 0x90, 0x90, 0x90 			// nop nop nop nop nop (5 bytes)
		, 0x90, 0x90, 0x90, 0x90 }			// nop nop nop nop (4 bytes)
	,	NULL
	,	15					// tot 15 bytes
	}
,	[LAST_ENTRY]=
	{	"Last entry"
	,	{ 0x00, 0x00, 0x00 }
	,	{ 0x00, 0x00, 0x00 }
	,	NULL
	,	3
	}
};


struct cheat_definition *get_definition(int cheat_id) {
	return &definitions[cheat_id];
}

void update_codecave_address
(	int def_seq
,	BYTE *address_in_jump_table
){
	BYTE *jump_code=definitions[def_seq].cheat_code // this is 0xFF 0x15
	,	*game_original_code_address = definitions[def_seq].destination_address // this is the place where the jump happens
	;
	if (jump_code[0] != 0xFF || jump_code[1] != 0x15) {
		file_log("%s : %d not a jump", __FILE__, __LINE__);
		return;
	}
	int64_t displacement = address_in_jump_table - (game_original_code_address + 6);
	if (displacement < INT32_MIN || displacement > INT32_MAX) {
		file_log("%s : %d overflow", __FILE__, __LINE__);
		return;
	}
	uint32_t value_32bit = (uint32_t)((int32_t)displacement);
	jump_code[2] = (value_32bit >> 0) & 0xFF;   // LSB
	jump_code[3] = (value_32bit >> 8) & 0xFF;
	jump_code[4] = (value_32bit >> 16) & 0xFF;
	jump_code[5] = (value_32bit >> 24) & 0xFF;  // MSB
}


