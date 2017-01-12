// GameSuspenderWrapper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GameSuspenderWrapper.h"
#define MAXCMDLINE 1024
#ifdef _DEBUG
	#define POLL_TIMER_MS 1000
#else
	#define POLL_TIMER_MS 200
#endif

STARTUPINFO si;
PROCESS_INFORMATION pi;

VOID CALLBACK TargetProcessExitCallback(
	_In_ PVOID   lpParameter,
	_In_ BOOLEAN TimerOrWaitFired
) {
	exit(0);
}

VOID CALLBACK TimerProc(
	_In_ HWND     hwnd,
	_In_ UINT     uMsg,
	_In_ UINT_PTR idEvent,
	_In_ DWORD    dwTime
) {
	const int fgWindowTitleBufLen = 500;
	wchar_t fgWindowTitleBuffer[fgWindowTitleBufLen + 1]; // Add 1 for zero termination, though I dunno if I need it
	LPWSTR fgWindowTitle = fgWindowTitleBuffer;

	HWND fgWindow = GetForegroundWindow();
	if (fgWindow == NULL) 
	{
		printf("GetForegroundWindow() is NULL\n\n");
	}
	else 
	{
		GetWindowTextW(fgWindow, fgWindowTitle, fgWindowTitleBufLen);
		printf("GetForegroundWindow() is: %S\n\n", fgWindowTitle);
	}
	DWORD dwProcessId;
	LPDWORD lpdwProcessId = &dwProcessId;
	GetWindowThreadProcessId(fgWindow, lpdwProcessId);

	if (dwProcessId != (DWORD)pi.dwProcessId && fgWindow != NULL)
	{
		printf("DebugActiveProcess PID: %d\n\n", pi.dwProcessId);
		DebugActiveProcess(pi.dwProcessId);
	}
	else
	{
		printf("DebugActiveProcessStop PID: %d\n\n", pi.dwProcessId);
		DebugActiveProcessStop(pi.dwProcessId);
	}
}

int CmdLineConcat(char *outStr, unsigned int argc, char *argv[])
{
	unsigned int i;
	size_t len = 0;
	char *_all_args, *all_args;

	for (i = 1; i<argc; i++) {
		len += strlen(argv[i]);
	}

	_all_args = all_args = (char *)malloc(len + argc - 1);

	for (i = 1; i<argc; i++) {
		memcpy(_all_args, argv[i], strlen(argv[i]));
		_all_args += strlen(argv[i]) + 1;
		*(_all_args - 1) = ' ';
	}
	*(_all_args - 1) = 0;

	printf("All %d args: '%s'\n", argc, all_args);

	strcpy_s(outStr, MAXCMDLINE, all_args);

	free(all_args);

	return 0;
}
int main(int argc, char *argv[])
{
	printf("This program was called with  \"%s\". \n", argv[1]);
	printf("\n");

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (argc <= 2)
	{
		printf("Usage: %s [cmdline]\n", argv[0]);
		return 1;
	}

	// Concatinate the command line arguments
	char *cmdLineConcat = (char *)malloc(MAXCMDLINE);
	if (cmdLineConcat == NULL) return -1;
	CmdLineConcat(cmdLineConcat, argc, argv);

	// Convert the command line parameter to LPWSTR
	// TODO: Might not be safe
	wchar_t commandLine[MAXCMDLINE * sizeof(wchar_t)]; // The sizeof part is probably an error!
	mbstowcs_s(NULL, commandLine, cmdLineConcat, strlen(cmdLineConcat) + 1); //Plus null
	LPWSTR commandLinePtr = commandLine;

	// Start the child process. 
	// TODO: We probably want to allow handles to be inheritable
	if (!CreateProcessW(NULL,   // No module name (use command line)
		commandLinePtr,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
		return 2;
	}
	SetTimer(NULL, 0x812, POLL_TIMER_MS, (TIMERPROC)TimerProc);
	// TODO: Probably too much access being requested here
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, true, pi.dwProcessId);

	// Loop until process exits
	/*DebugActiveProcess(pi.dwProcessId);
	DebugActiveProcessStop(pi.dwProcessId);*/

	// Wait until child process exits.
	
	PHANDLE *phWaitObject;
	if (!RegisterWaitForSingleObject(
		(PHANDLE) &phWaitObject,
		processHandle,
		TargetProcessExitCallback,
		processHandle,
		INFINITE,
		WT_EXECUTELONGFUNCTION && WT_EXECUTEONLYONCE
	))
	{
		printf("RegisterWaitForSingleObject failed (%d).\n", GetLastError());
		return 3;
	}

	MSG msg;

#ifdef _DEBUG
	printf(" === !!! THIS IS A DEBUG BUILD. POLLING IS AT %i ms !!! ===\n\n", POLL_TIMER_MS);
#endif // _DEBUG
	while (true) {
		GetMessage(&msg, NULL, 0, 0);
		DispatchMessage(&msg);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return 0;
}

