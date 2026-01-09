#ifndef LOGIC_H_
#define LOGIC_H_

BOOL	do_cheat(char *, int);
BYTE	*get_base_address();
int	debug_tests();
int	file_log(char* format, ...);
int	is_target_loaded();
int	perform_action(int, bool);
int	show_error_return_false( TCHAR* msg);
int	wait_for_process_and_inject();
void	init_jump_table();


#define TARGET_EXE "GenerationZero_F.exe"
// #define TARGET_EXE "Notepad.exe" // TESTS

typedef struct {
	WCHAR log_file_path[MAX_PATH];
	DWORD injector_pid;
	DWORD magic_number;  // For validation
} DLL_PARAMS; // we only have one parameter

#define PARAM_MAGIC 0xCED1FAB0

#endif
