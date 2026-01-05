
#include <windows.h>
#include <stdint.h>
#include "codecaves.h"
#include "getbaseaddress.h"
#include "logic.h"
#include "definitions.h"


float	g_y_acceleration=1
,	g_speed=1
,	g_PAIN=50
,	l_TICK=0;
;


// GCC-style inline assembly syntax breakdown
// void easy_kill_codecave(){
// __asm__(
// 	"xorps  %xmm0,%xmm0;" // 0F57 C0
// 	"movss  %xmm0,0x11c(%ebx); (9) " // 0x67, 0xF3, 0x0F, 0x11, 0x83, 0x1C, 0x01, 0x00, 0x00
// 	"ret;"
// );
// }


void increase_time_gap(){
l_TICK+=0.01;
	file_log( "%s : %d TICK now `%f`", __FILE__, __LINE__, l_TICK );
}
void reset_time_gap() {
l_TICK=0;
	file_log( "%s : %d TICK now `%f`", __FILE__, __LINE__, l_TICK );
}

void move_clock_codecave (){
__asm__(
	"movaps %%xmm6,%%xmm1;"
	"addss  %0,%%xmm0;" // Add Scalar Single-precision: xmm0 = xmm0 + parameter below
	"addss  %%xmm0,%%xmm1;"
	"ret;"
	:
	: "m" (l_TICK)
//      |  |     └─── C variable/expression
//      |  └───────── Constraint ("m" = 'memory operand')
//      └──────────── Colon separating assembly template from inputs
);
}

void zero_weight_codecave (){
__asm__(
	"movabs $0x2edbe6ff,%rbx;"
	"movq   %rbx,%xmm0;"
	"movss  %xmm0,0x12c(%rax);"
	"ret;"
);
}



void super_speed_codecave(){
__asm__(
	"mulss  %0,%%xmm6;"	//	multiply xmm6 by parameter
	"movss  0x18(%%r14),%%xmm7;"
	"ret;"
	:
	: "m" (g_speed)
);
}


void flying_codecave(){
__asm__(
	"movss  0x44(%%rdi),%%xmm2;"	// put rdi+68 in xmm2
	"addss  %0,%%xmm2;"		// add g_y_acceleration to xmm2
	"movss  %%xmm2,0x44(%%rdi);"	// result goes to rdi+0x44
	"movaps %%xmm6,0x50(%%rdi);"	// original instruction
	"ret;"
	:
	: "m" (g_y_acceleration)
);
}


void reset_speed(){
	g_speed=0;
	file_log( "%s : %d SPEED now `%f`", __FILE__, __LINE__, g_speed );
}

void reset_acceleration_value(){
	g_y_acceleration=5;
	file_log( "%s : %d ACCELERATION now `%f`", __FILE__, __LINE__, g_y_acceleration );
}


void increase_speed(){
	g_speed+=1;
	file_log( "%s : %d SPEED now `%f`", __FILE__, __LINE__, g_speed );
}

void increase_acceleration_value(){
	g_y_acceleration+=10;
	file_log( "%s : %d ACCELERATION now `%f`", __FILE__, __LINE__, g_y_acceleration );
}



void reset_pain(){
	g_PAIN=0;
	file_log( "%s : %d pain now `%f`", __FILE__, __LINE__, g_PAIN );
}
void increase_pain(){
	g_PAIN+=10;
	file_log( "%s : %d pain now `%f`", __FILE__, __LINE__, g_PAIN );
}



void update_codecave_address
(	int def_seq
,	BYTE *address_in_jump_table
){
	BYTE *jump_code=get_definition(def_seq)->cheat_code // this is 0xFF 0x15
	,	*game_original_code_address = get_definition(def_seq)->destination_address // this is the place where the jump happens
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
	file_log("%s : %d updated codecave '%s'", __FILE__, __LINE__, get_definition(def_seq)->cheat_prompt);
}



void update_codecave_addresses(){
	BYTE	*base_address=getBaseAddress()
	,	*jump_table=base_address+0x450
	,	*where_is_run_codecave=jump_table+0x08
	,	*where_is_tick_codecave=jump_table+0x18
	,	*where_is_one_shot_kill_cc=jump_table
	;
	update_codecave_address
	(	LETS_TICK
	,	where_is_tick_codecave
	);
	update_codecave_address
	(	LETS_RUN
	,	where_is_run_codecave
	);
	update_codecave_address
	(	LETS_KILL
	,	where_is_one_shot_kill_cc
	);
}



