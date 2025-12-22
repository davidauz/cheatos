#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

#include <stdbool.h>
#include "logic.h"
#include "definitions.h"

#define CHECK_ARGUMENT \
if(argc<=(1+idx)) \
	return show_error_exit("Missing argument after option `%s`\n", argv[idx]);

#define OPTION_DELETE_LOG_FILE 0x01
#define OPTION_LOG_FILE 0x20

WCHAR g_log_file_name[MAX_PATH]={0};

void prepare_shared_memory()
{
// pass parameters to the DLL
	DLL_PARAMS shared_memory_local_copy = {0};
	HANDLE hMapFile = CreateFileMapping
	(	INVALID_HANDLE_VALUE
	,	NULL
	,	PAGE_READWRITE
	, 	0
	,	sizeof(DLL_PARAMS)
	,	"Global\\MyDLLParams"
	);

	LPVOID p_buffer_for_parameters = MapViewOfFile(hMapFile, FILE_MAP_WRITE, 0, 0, sizeof(DLL_PARAMS));
	DLL_PARAMS* p_dll_params_in_shared_memory = (DLL_PARAMS*)p_buffer_for_parameters;
	wcscpy(p_dll_params_in_shared_memory->log_file_path, g_log_file_name);
	p_dll_params_in_shared_memory->injector_pid = GetCurrentProcessId();
	p_dll_params_in_shared_memory->magic_number = PARAM_MAGIC;

	UnmapViewOfFile(p_buffer_for_parameters);
}



int show_error_exit(char* format, ...){
	DWORD error = GetLastError();

	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);

	LPSTR messageBuffer = 0;
	size_t size = FormatMessageA
	(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
	,	NULL
	,	error
	,	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
	,	(LPSTR)&messageBuffer
	,	0
	,	NULL
	);

	if (size > 0 && messageBuffer != NULL) {
		file_log("%s", messageBuffer);
		LocalFree(messageBuffer);
	} else {
		file_log("System error (%lu): Unknown error\n", error);
	}
	
	return ERROR_VALUE;
}


int Usage(){
	return file_log("%s:%d\nUsage\n\n"
"-h: this help\n"
"-f <log file>: log file name (default '%s')\n"
"-r: delete log file contents at startup\n"
"\n"
,	__FILE__
,	__LINE__
,	g_log_file_name
);
}


int WINAPI WinMain (	HINSTANCE hInstance
,	HINSTANCE hPrevInstance
,	char *lpCmdLine
,	int nCmdShow
) {
	int argc = 0
	,	CL_OPTIONS=0;
	LPWSTR* argv = NULL;
	LPWSTR cmdLine = GetCommandLineW();
	argv = CommandLineToArgvW(cmdLine, &argc);
	int idx=1; // 0 is the executable
	wcscpy(g_log_file_name, L"c:\\LOG.txt"); // default

	while (idx < argc ) {
		if(!lstrcmpiW(L"-h", argv[idx]))
			return Usage();			
		else if(!lstrcmpiW(L"-f", argv[idx])){
			CHECK_ARGUMENT
			CL_OPTIONS |= OPTION_LOG_FILE;
			if(MAX_PATH<lstrlenW(argv[++idx]))
				return show_error_exit("Log file max path length is 37 bytes\n");
			wcscpy(g_log_file_name, argv[idx++]);
		} else if(!lstrcmpiW(L"-r", argv[idx])){
			CL_OPTIONS |= OPTION_DELETE_LOG_FILE;
			idx++;
		} else
			return show_error_exit("%s:%d unknown option `%s`\n", __FILE__, __LINE__, argv[idx]);
	}

	prepare_shared_memory();

	wait_for_process_and_inject();
	return 0;
}

