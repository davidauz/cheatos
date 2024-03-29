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

void	reset_acceleration_value();
void	reset_speed();
void	reset_pain();
void	reset_time_gap();

void	increase_acceleration_value();
void	increase_speed();
void	increase_pain();
void	increase_time_gap();



#endif
