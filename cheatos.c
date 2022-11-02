#include <windows.h>
#include <stdbool.h>

#include "logic.h"

// https://github.com/microsoft/Windows-classic-samples/tree/main/Samples
// https://www.nexus-6.uk/push-button-control/
#define	ID_INFINITE_LIVES 2
#define	ID_INFINITE_AMMO  3

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain
(	HINSTANCE hInstance
,	HINSTANCE hPrevInstance
,	PWSTR lpCmdLine
,	int nCmdShow
) {
	MSG  msg;    
	WNDCLASSW wc = {0};
	wc.lpszClassName = L"Cheatos";
	wc.hInstance     = hInstance ;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc   = WndProc;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);

	if ( ! is_target_loaded() ) {
		MessageBox
		(	NULL //   [in, optional] HWND    hWnd
		,	L"Target not loaded" //   [in, optional] LPCTSTR lpText
		,	L"Error" // [in, optional] LPCTSTR lpCaption
		,	MB_OK // [in]           UINT    uType
		);
		return 255;
	}

	RegisterClassW(&wc);
	CreateWindowW
	(	wc.lpszClassName // Predefined class; Unicode assumed 
	,	L"Cheatos" // text 
	,	WS_OVERLAPPEDWINDOW | WS_VISIBLE // Styles 
	,	150 // x position 
	,	150 // y position 
	,	230 // width
	,	150 // height
	,	0 // Parent window
	,	0 // No menu.
	,	hInstance // hInstance from winmain
	,	0 // [in, optional] lpParam
	);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc
(	HWND hwnd
,	UINT msg
,	WPARAM wParam
,	LPARAM lParam
) {
	bool checked = true;

	switch(msg) {
		case WM_CREATE:
			CreateWindowW
			(	L"button"
			,	L"Infinite ammo"
			,	WS_VISIBLE | WS_CHILD | BS_CHECKBOX
			,	20 // x
			,	20 // y
			,	185 // w
			,	35 // h
			,	hwnd
			,	(HMENU) ID_INFINITE_AMMO
			,	NULL
			,	NULL
			);
			CheckDlgButton(hwnd, 1, BST_UNCHECKED);

			CreateWindowW
			(	L"button"
			,	L"Infinite lives"
			,	WS_VISIBLE | WS_CHILD | BS_CHECKBOX
			,	20 // x
			,	60 // y
			,	185 // w
			,	35 // h
			,	hwnd
			,	(HMENU) ID_INFINITE_LIVES
			,	NULL
			,	NULL
			);
			CheckDlgButton(hwnd, 1, BST_UNCHECKED);
		break;

		case WM_COMMAND:
			if (ID_INFINITE_LIVES == wParam) {
				checked = IsDlgButtonChecked(hwnd, ID_INFINITE_LIVES);
				if (checked) {
					CheckDlgButton(hwnd, ID_INFINITE_LIVES, BST_UNCHECKED);
					do_cheat("LIVES", 0);
				} else {
					CheckDlgButton(hwnd, ID_INFINITE_LIVES, BST_CHECKED);
					do_cheat("LIVES", 1);
				}
			} else if (ID_INFINITE_AMMO == wParam ) {
				checked = IsDlgButtonChecked(hwnd, ID_INFINITE_AMMO);
				if (checked) {
					CheckDlgButton(hwnd, ID_INFINITE_AMMO, BST_UNCHECKED);
					do_cheat("AMMO", 0);
				} else {
					CheckDlgButton(hwnd, ID_INFINITE_AMMO, BST_CHECKED);
					do_cheat("AMMO", 1);
				}
			} else {
				TCHAR buff[100];
				wsprintf(buff, L"`%d` ; `%d`", wParam, ID_INFINITE_LIVES);
				MessageBox
				(	NULL //   [in, optional] HWND    hWnd
				,	buff //   [in, optional] LPCTSTR lpText
				,	L"PEPPE" // [in, optional] LPCTSTR lpCaption
				,	MB_OK // [in]           UINT    uType
				);
			}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
		break;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

