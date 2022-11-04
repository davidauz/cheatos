#include <windows.h>
#include <stdbool.h>

#include "definitions.h"
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
	unsigned int curr_ID=0;

	switch(msg) {
		case WM_CREATE:
			int	curr_y=20
			;
			for(unsigned int i=0; i < sizeof(definitions)/sizeof(definitions[0]); i++) {
				curr_ID=1+i;
				CreateWindowW
				(	L"button"
				,	definitions[i].cheat_prompt
				,	WS_VISIBLE | WS_CHILD | BS_CHECKBOX
				,	20 // x
				,	curr_y // y
				,	185 // w
				,	35 // h
				,	hwnd
				,	(HMENU)(UINT_PTR) curr_ID
				,	NULL
				,	NULL
				);
				CheckDlgButton(hwnd, curr_ID, BST_UNCHECKED);
				curr_y +=40;
			}
		break;

		case WM_COMMAND:
			checked = IsDlgButtonChecked(hwnd, wParam);
			perform_action(wParam-1, !checked);
			CheckDlgButton(hwnd, wParam, checked?BST_UNCHECKED:BST_CHECKED);
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
		break;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

