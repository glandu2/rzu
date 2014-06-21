#include "CrashHandler.h"
#include "Log.h"
#include "EventLoop.h"

//if equal to 1, don't do a crashdump
static long long int dumpMode = 0;
uv_async_t CrashHandler::asyncCallback;
void* CrashHandler::callbackInstance = nullptr;

void CrashHandler::setDumpMode(int _dumpMode) {
	dumpMode = _dumpMode;
}

void CrashHandler::setTerminateCallback(TerminateCallback callback, void* instance) {
	asyncCallback.data = (void*)callback;
	callbackInstance = instance;
}

void CrashHandler::onTerminate(uv_async_t *) {
	TerminateCallback callback = (TerminateCallback) asyncCallback.data;
	if(callback)
		callback(callbackInstance);
	else
		exit(1);
}

#ifdef _WIN32

#include <windows.h>
#include <dbghelp.h>
#include <new.h>
#include <signal.h>
#include <exception>

//#include <stdio.h>
//#include <conio.h>
//#include <exception>
//#include <sys/stat.h>
//#include <psapi.h>
//#include <rtcapi.h>
//#include <Shellapi.h>

// Collects current process state.
static void getExceptionPointers(
		DWORD dwExceptionCode,
		EXCEPTION_POINTERS** pExceptionPointers);

// This method creates minidump of the process
static void createMiniDump(EXCEPTION_POINTERS* pExcPtrs);

/* Exception handler functions. */

static LONG WINAPI sehHandler(PEXCEPTION_POINTERS pExceptionPtrs);
static void __cdecl terminateHandler();
static void __cdecl unexpectedHandler();

static void __cdecl pureCallHandler();

static void __cdecl invalidParameterHandler(const wchar_t* expression,
											const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);

static int __cdecl newHandler(size_t);

static void sigabrtHandler(int);
static void sigfpeHandler(int /*code*/, int subcode);
//static void sigintHandler(int);
static void sigillHandler(int);
static void sigsegvHandler(int);
static void sigtermHandler(int);
static BOOL WINAPI handlerRoutine(DWORD dwCtrlType);

#ifndef _AddressOfReturnAddress

// Taken from: http://msdn.microsoft.com/en-us/library/s975zw7k(VS.71).aspx
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

// _ReturnAddress and _AddressOfReturnAddress should be prototyped before use
EXTERNC void * _AddressOfReturnAddress(void);
EXTERNC void * _ReturnAddress(void);

#endif


void CrashHandler::setProcessExceptionHandlers()
{
	uv_async_init(EventLoop::getLoop(), &asyncCallback, &onTerminate);
	uv_unref((uv_handle_t*)&asyncCallback);
	// Install top-level SEH handler
	SetUnhandledExceptionFilter(&sehHandler);
	SetConsoleCtrlHandler(&handlerRoutine, TRUE);

	// Catch pure virtual function calls.
	// Because there is one _purecall_handler for the whole process,
	// calling this function immediately impacts all threads. The last
	// caller on any thread sets the handler.
	// http://msdn.microsoft.com/en-us/library/t296ys27.aspx
	_set_purecall_handler(&pureCallHandler);

	// Catch new operator memory allocation exceptions
#ifndef __MINGW32__
	_set_new_handler(&newHandler);
#endif

	// Catch invalid parameter exceptions.
	_set_invalid_parameter_handler(&invalidParameterHandler);

	// Set up C++ signal handlers
#ifndef __MINGW32__
	_set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
#endif

	// Catch an abnormal program termination
	signal(SIGABRT, &sigabrtHandler);

	// Catch illegal instruction handler
	//signal(SIGINT, &sigintHandler);

	// Catch a termination request
	signal(SIGTERM, &sigtermHandler);

}

void CrashHandler::setThreadExceptionHandlers()
{

	// Catch terminate() calls.
	// In a multithreaded environment, terminate functions are maintained
	// separately for each thread. Each new thread needs to install its own
	// terminate function. Thus, each thread is in charge of its own termination handling.
	// http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
	std::set_terminate(&terminateHandler);

	// Catch unexpected() calls.
	// In a multithreaded environment, unexpected functions are maintained
	// separately for each thread. Each new thread needs to install its own
	// unexpected function. Thus, each thread is in charge of its own unexpected handling.
	// http://msdn.microsoft.com/en-us/library/h46t5b69.aspx
	std::set_unexpected(&unexpectedHandler);

	// Catch a floating point error
	typedef void (*sigh)(int);
	signal(SIGFPE, (sigh)sigfpeHandler);

	// Catch an illegal instruction
	signal(SIGILL, &sigillHandler);

	// Catch illegal storage access errors
	signal(SIGSEGV, &sigsegvHandler);

}

// The following code gets exception pointers using a workaround found in CRT code.
void getExceptionPointers(DWORD dwExceptionCode,
						  EXCEPTION_POINTERS** ppExceptionPointers)
{
	// The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

	EXCEPTION_RECORD ExceptionRecord;
	CONTEXT ContextRecord;
	memset(&ContextRecord, 0, sizeof(CONTEXT));

#if defined (_X86_) || defined (_IA64_) || defined (_AMD64_)
	RtlCaptureContext(&ContextRecord);

#else  /* defined (_IA64_) || defined (_AMD64_) */

	ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif  /* defined (_IA64_) || defined (_AMD64_) */

	ZeroMemory(&ExceptionRecord, sizeof(EXCEPTION_RECORD));

	ExceptionRecord.ExceptionCode = dwExceptionCode;

#ifdef __MINGW32__
	ExceptionRecord.ExceptionAddress = __builtin_return_address(0);
#else
	ExceptionRecord.ExceptionAddress = _ReturnAddress();
#endif

	///

	EXCEPTION_RECORD* pExceptionRecord = new EXCEPTION_RECORD;
	memcpy(pExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));
	CONTEXT* pContextRecord = new CONTEXT;
	memcpy(pContextRecord, &ContextRecord, sizeof(CONTEXT));

	*ppExceptionPointers = new EXCEPTION_POINTERS;
	(*ppExceptionPointers)->ExceptionRecord = pExceptionRecord;
	(*ppExceptionPointers)->ContextRecord = pContextRecord;
}

// This method creates minidump of the process
void createMiniDump(EXCEPTION_POINTERS* pExcPtrs)
{
	HANDLE hFile = NULL;
	MINIDUMP_EXCEPTION_INFORMATION mei;
	MINIDUMP_CALLBACK_INFORMATION mci;

	//if equal to 1, don't do a crashdump
	if(dumpMode == 1)
		return;

	// Create the minidump file
	hFile = CreateFileA(
				"crashdump.dmp",
				GENERIC_WRITE,
				0,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		// Couldn't create file
		char szScratch[_MAX_PATH*2];
		wsprintf(szScratch, "Failed to open dump file \'crashdump.dmp\' (error %d)", GetLastError());
		OutputDebugString(szScratch);
		return;
	}

	// Write minidump to the file
	mei.ThreadId = GetCurrentThreadId();
	mei.ExceptionPointers = pExcPtrs;
	mei.ClientPointers = FALSE;

	mci.CallbackRoutine = NULL;
	mci.CallbackParam = NULL;

	HANDLE hProcess = GetCurrentProcess();
	DWORD dwProcessId = GetCurrentProcessId();

	BOOL bWriteDump = MiniDumpWriteDump(
				hProcess,
				dwProcessId,
				hFile,
				MiniDumpWithDataSegs,
				&mei,
				NULL,
				&mci);

	if(!bWriteDump)
	{
		char szScratch[_MAX_PATH*2];
		wsprintf(szScratch, "Failed to save dump file to \'crashdump.dmp\' (error %d)", GetLastError());
		OutputDebugString(szScratch);
	}

	// Close file
	CloseHandle(hFile);

	//Stop and flush log message. Do it after crash dump, this might crash ...
	Log::get()->stopWriter();
}

// Structured exception handler
LONG WINAPI sehHandler(PEXCEPTION_POINTERS pExceptionPtrs)
{
	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);

	// Unreacheable code
	return EXCEPTION_EXECUTE_HANDLER;
}

// CRT terminate() call handler
void __cdecl terminateHandler()
{
	// Abnormal program termination (terminate() function was called)

	// Retrieve exception information
	EXCEPTION_POINTERS* pExceptionPtrs = NULL;
	getExceptionPointers(0, &pExceptionPtrs);

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);
}

// CRT unexpected() call handler
void __cdecl unexpectedHandler()
{
	// Unexpected error (unexpected() function was called)

	// Retrieve exception information
	EXCEPTION_POINTERS* pExceptionPtrs = NULL;
	getExceptionPointers(0, &pExceptionPtrs);

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);
}

// CRT Pure virtual method call handler
void __cdecl pureCallHandler()
{
	// Pure virtual function call

	// Retrieve exception information
	EXCEPTION_POINTERS* pExceptionPtrs = NULL;
	getExceptionPointers(0, &pExceptionPtrs);

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);

}


// CRT invalid parameter handler
void __cdecl invalidParameterHandler(
		const wchar_t* expression,
		const wchar_t* function,
		const wchar_t* file,
		unsigned int line,
		uintptr_t pReserved)
{
	pReserved;

	// Invalid parameter exception

	// Retrieve exception information
	EXCEPTION_POINTERS* pExceptionPtrs = NULL;
	getExceptionPointers(0, &pExceptionPtrs);

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);

}

// CRT new operator fault handler
int __cdecl newHandler(size_t)
{
	// 'new' operator memory allocation exception

	// Retrieve exception information
	EXCEPTION_POINTERS* pExceptionPtrs = NULL;
	getExceptionPointers(0, &pExceptionPtrs);

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);

	// Unreacheable code
	return 0;
}

// CRT SIGABRT signal handler
void sigabrtHandler(int)
{
	// Caught SIGABRT C++ signal

	// Retrieve exception information
	EXCEPTION_POINTERS* pExceptionPtrs = NULL;
	getExceptionPointers(0, &pExceptionPtrs);

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);

}

// CRT SIGFPE signal handler
void sigfpeHandler(int /*code*/, int subcode)
{
	// Floating point exception (SIGFPE)

	EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);

}

// CRT sigill signal handler
void sigillHandler(int)
{
	// Illegal instruction (SIGILL)

	// Retrieve exception information
	EXCEPTION_POINTERS* pExceptionPtrs = NULL;
	getExceptionPointers(0, &pExceptionPtrs);

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);

}

// CRT sigint signal handler
/*void sigintHandler(int)
{
	// Interruption (SIGINT)

	// Retrieve exception information
	EXCEPTION_POINTERS* pExceptionPtrs = NULL;
	getExceptionPointers(0, &pExceptionPtrs);

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);

}*/

// CRT SIGSEGV signal handler
void sigsegvHandler(int)
{
	// Invalid storage access (SIGSEGV)

	PEXCEPTION_POINTERS pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);

}

// CRT SIGTERM signal handler
void sigtermHandler(int)
{
	// Termination request (SIGTERM)
/*
	// Retrieve exception information
	EXCEPTION_POINTERS* pExceptionPtrs = NULL;
	getExceptionPointers(0, &pExceptionPtrs);

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);*/
	CrashHandler::onTerminate();

}

static BOOL WINAPI handlerRoutine(DWORD dwCtrlType) {
	if(dwCtrlType == CTRL_CLOSE_EVENT || dwCtrlType == CTRL_BREAK_EVENT || dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_SHUTDOWN_EVENT) {
		CrashHandler::terminate();
		//wait end
		Sleep(5000);
		return TRUE;
	}
	return FALSE;
}

#else /* _WIN32 */

//UNIX

#include <signal.h>

static void onSignal(int sig) {
	CrashHandler::terminate();
}

void CrashHandler::setProcessExceptionHandlers() {
	struct sigaction sa;

	uv_async_init(EventLoop::getLoop(), &asyncCallback, &onTerminate);
	uv_unref((uv_handle_t*)&asyncCallback);

	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = &onSignal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
}

void CrashHandler::setThreadExceptionHandlers() {

}

#endif
