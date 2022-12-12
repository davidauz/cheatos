#include <windows.h>

#include <stdbool.h>
#include "logic.h"

int WINAPI WinMain (	HINSTANCE hInstance
,	HINSTANCE hPrevInstance
,	char *lpCmdLine
,	int nCmdShow
) {
	wait_for_process_and_inject();
	return 0;
}

