#include <windows.h>

#include <stdbool.h>
#include "logic.h"

/*
void showError() {
	TCHAR buff[100];
	wsprintf(buff, L"`%d` ; `%d`", wParam, ID_INFINITE_LIVES);
	MessageBox
	(	NULL //   [in, optional] HWND    hWnd
	,	buff //   [in, optional] LPCTSTR lpText
	,	L"PEPPE" // [in, optional] LPCTSTR lpCaption
	,	MB_OK // [in]           UINT    uType
	);
}
*/

int WINAPI wWinMain (	HINSTANCE hInstance
,	HINSTANCE hPrevInstance
,	PWSTR lpCmdLine
,	int nCmdShow
) {
	wait_for_process_and_inject();
	return 0;
}

