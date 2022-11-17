#ifndef LOGIC_H_
#define LOGIC_H_


int is_target_loaded();
BOOL do_cheat(char *, int);
int show_error_return_false( TCHAR* msg);
int perform_action(int, bool);
int wait_for_process_and_inject();

#endif
