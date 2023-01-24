#ifndef LOGIC_H_
#define LOGIC_H_

int is_target_loaded();
BOOL do_cheat(char *, int);
int show_error_return_false( TCHAR* msg);
int perform_action(int, bool);
int wait_for_process_and_inject();
int debug_tests();
int file_log(char* format, ...);
void increase_acceleration_value();
void reset_acceleration_value();
void init_jump_table();
BYTE *get_base_address();

#endif
