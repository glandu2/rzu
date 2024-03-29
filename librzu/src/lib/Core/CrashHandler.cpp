#include "CrashHandler.h"
#include "ClassCounter.h"
#include "Config/GlobalCoreConfig.h"
#include "Console/ConsoleCommands.h"
#include "EventLoop.h"
#include "Log.h"
#include <map>
#include <stdlib.h>

#ifdef __GLIBC__
#include <malloc.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <crtdbg.h>
#include <dbghelp.h>
#include <exception>
#include <malloc.h>
#include <new.h>
#include <signal.h>
#include <windows.h>
#endif

// if equal to 1, don't do a crashdump
static long long int dumpMode = 0;
uv_async_t CrashHandler::asyncCallback;
void* CrashHandler::callbackInstance = nullptr;
bool CrashHandler::interruptAttemptInProgress = false;
bool CrashHandler::globalHandlersInitialized = false;

void CrashHandler::init() {
	uv_async_init(EventLoop::getLoop(), &asyncCallback, &onTerminate);
	uv_unref((uv_handle_t*) &asyncCallback);

	if(globalHandlersInitialized == false) {
		globalHandlersInitialized = true;

		setProcessExceptionHandlers();

		ConsoleCommands::get()->addCommand("terminate",
		                                   "term",
		                                   0,
		                                   0,
		                                   &commandTerminate,
		                                   "Ask the server to terminate gracefuly",
		                                   "terminate : terminate the server gracefuly");

		ConsoleCommands::get()->addCommand("mem.list", "mem", 0, 0, &commandListObjectsCount, "List objects count");
	}
	setThreadExceptionHandlers();
	GlobalCoreConfig::get()->admin.dumpMode.addListener(nullptr, &setDumpMode);
	setDumpMode(nullptr, &GlobalCoreConfig::get()->admin.dumpMode);
}

void CrashHandler::deinit() {
	uv_ref((uv_handle_t*) &asyncCallback);
	uv_close((uv_handle_t*) &asyncCallback, nullptr);
}

void CrashHandler::setDumpMode(IListener*, cval<int>*) {
	dumpMode = GlobalCoreConfig::get()->admin.dumpMode.get();
}

void CrashHandler::setTerminateCallback(TerminateCallback callback, void* instance) {
	asyncCallback.data = (void*) callback;
	callbackInstance = instance;
}

void CrashHandler::terminate() {
	if(!interruptAttemptInProgress) {
		interruptAttemptInProgress = true;
		uv_async_send(&asyncCallback);
	} else {
		exit(1);
	}
}

void CrashHandler::onTerminate(uv_async_t*) {
	TerminateCallback callback = (TerminateCallback) asyncCallback.data;
	if(callback)
		callback(callbackInstance);
	else
		exit(1);
}

void CrashHandler::commandTerminate(IWritableConsole*, const std::vector<std::string>&) {
	terminate();
}

void CrashHandler::commandListObjectsCount(IWritableConsole* console, const std::vector<std::string>&) {
	std::map<std::string, unsigned long*>::const_iterator it, itEnd;

#ifdef __GLIBC__
#if __GLIBC_PREREQ(2, 33)
	struct mallinfo2 memUsage = mallinfo2();
#else
	struct mallinfo memUsage = mallinfo();
#endif
	console->writef("Memory usage:\r\n"
	                " heap size: %zu\r\n"
	                " unused chunks: %zu\r\n"
	                " mmap chunks: %zu\r\n"
	                " mmap mem size: %zu\r\n"
	                " used mem size: %zu\r\n"
	                " unused mem size: %zu\r\n"
	                " trailing releasable size: %zu\r\n\r\n",
	                (size_t) memUsage.arena,
	                (size_t) memUsage.ordblks,
	                (size_t) memUsage.hblks,
	                (size_t) memUsage.hblkhd,
	                (size_t) memUsage.uordblks,
	                (size_t) memUsage.fordblks,
	                (size_t) memUsage.keepcost);
#endif

#if defined(_WIN32) && defined(_DEBUG)
	_CrtMemState memUsage;
	memset(&memUsage, 0, sizeof(memUsage));
	_CrtMemDumpStatistics(&memUsage);
	console->writef("Memory usage:\r\n"
	                " normal block size: %ld\r\n"
	                " free block size: %ld\r\n"
	                " CRT block size: %ld\r\n"
	                " client block size: %ld\r\n"
	                " ignore block size: %ld\r\n"
	                " peak memory size: %ld\r\n"
	                " used memory size: %ld\r\n\r\n",
	                memUsage.lSizes[_NORMAL_BLOCK],
	                memUsage.lSizes[_FREE_BLOCK],
	                memUsage.lSizes[_CRT_BLOCK],
	                memUsage.lSizes[_CLIENT_BLOCK],
	                memUsage.lSizes[_IGNORE_BLOCK],
	                memUsage.lHighWaterCount,
	                memUsage.lTotalCount);
#endif

	for(it = getObjectsCount().cbegin(), itEnd = getObjectsCount().cend(); it != itEnd; ++it) {
		console->writef("%s: %ld\r\n", it->first.c_str(), *it->second);
	}
}

#ifdef _WIN32

// Collects current process state.
static void getExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS** pExceptionPointers);

// This method creates minidump of the process
static void createMiniDump(EXCEPTION_POINTERS* pExcPtrs);

/* Exception handler functions. */

static LONG WINAPI sehHandler(PEXCEPTION_POINTERS pExceptionPtrs);
static void __cdecl terminateHandler();
static void __cdecl unexpectedHandler();

static void __cdecl pureCallHandler();

static void __cdecl invalidParameterHandler(
    const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);

static int __cdecl newHandler(size_t);

static void sigabrtHandler(int);
static void sigfpeHandler(int /*code*/, int subcode);
// static void sigintHandler(int);
static void sigillHandler(int);
static void sigsegvHandler(int);
static void sigtermHandler(int);
static BOOL WINAPI handlerRoutine(DWORD dwCtrlType);

#ifndef _AddressOfReturnAddress

extern "C" void* _AddressOfReturnAddress(void);
extern "C" void* _ReturnAddress(void);

#endif

void CrashHandler::setProcessExceptionHandlers() {
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
	// signal(SIGINT, &sigintHandler);

	// Catch a termination request
	signal(SIGTERM, &sigtermHandler);
}

void CrashHandler::setThreadExceptionHandlers() {
	// Catch terminate() calls.
	// In a multithreaded environment, terminate functions are maintained
	// separately for each thread. Each new thread needs to install its own
	// terminate function. Thus, each thread is in charge of its own termination handling.
	// http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
	std::set_terminate(&terminateHandler);

	// Catch a floating point error
	typedef void (*sigh)(int);
	signal(SIGFPE, (sigh) sigfpeHandler);

	// Catch an illegal instruction
	signal(SIGILL, &sigillHandler);

	// Catch illegal storage access errors
	signal(SIGSEGV, &sigsegvHandler);
}

// The following code gets exception pointers using a workaround found in CRT code.
void getExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS** ppExceptionPointers) {
	// The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

	EXCEPTION_RECORD ExceptionRecord;
	CONTEXT ContextRecord;
	memset(&ContextRecord, 0, sizeof(CONTEXT));

#if defined(_X86_) || defined(_IA64_) || defined(_AMD64_)
	RtlCaptureContext(&ContextRecord);

#else /* defined (_IA64_) || defined (_AMD64_) */

	ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif /* defined (_IA64_) || defined (_AMD64_) */

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
void createMiniDump(EXCEPTION_POINTERS* pExcPtrs) {
	HANDLE hFile = NULL;
	MINIDUMP_EXCEPTION_INFORMATION mei;
	MINIDUMP_CALLBACK_INFORMATION mci;

	// if equal to 1, don't do a crashdump
	if(dumpMode == 1)
		return;

	// Create the minidump file
	hFile = CreateFileA("crashdump.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE) {
		// Couldn't create file
		char szScratch[_MAX_PATH * 2];
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

	BOOL bWriteDump = MiniDumpWriteDump(hProcess,
	                                    dwProcessId,
	                                    hFile,
	                                    MINIDUMP_TYPE(MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory),
	                                    &mei,
	                                    NULL,
	                                    &mci);

	if(!bWriteDump) {
		char szScratch[_MAX_PATH * 2];
		wsprintf(szScratch, "Failed to save dump file to \'crashdump.dmp\' (error %d)", GetLastError());
		OutputDebugString(szScratch);
	}

	// Close file
	CloseHandle(hFile);

	// Stop and flush log message. Do it after crash dump, this might crash ...
	if(Log::get())
		Log::get()->stopWriter();
}

// Structured exception handler
LONG WINAPI sehHandler(PEXCEPTION_POINTERS pExceptionPtrs) {
	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);

	// Unreacheable code
	return EXCEPTION_EXECUTE_HANDLER;
}

// CRT terminate() call handler
void __cdecl terminateHandler() {
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
void __cdecl unexpectedHandler() {
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
void __cdecl pureCallHandler() {
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
    const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved) {
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
static int __cdecl newHandler(size_t) {
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
void sigabrtHandler(int) {
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
void sigfpeHandler(int /*code*/, int subcode) {
	// Floating point exception (SIGFPE)

	EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS) _pxcptinfoptrs;

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);
}

// CRT sigill signal handler
void sigillHandler(int) {
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
void sigsegvHandler(int) {
	// Invalid storage access (SIGSEGV)

	PEXCEPTION_POINTERS pExceptionPtrs = (PEXCEPTION_POINTERS) _pxcptinfoptrs;

	// Write minidump file
	createMiniDump(pExceptionPtrs);

	// Terminate process
	TerminateProcess(GetCurrentProcess(), 1);
}

// CRT SIGTERM signal handler
void sigtermHandler(int) {
	// Termination request (SIGTERM)
	/*
	    // Retrieve exception information
	    EXCEPTION_POINTERS* pExceptionPtrs = NULL;
	    getExceptionPointers(0, &pExceptionPtrs);

	    // Write minidump file
	    createMiniDump(pExceptionPtrs);

	    // Terminate process
	    TerminateProcess(GetCurrentProcess(), 1);*/
	CrashHandler::terminate();
}

static BOOL WINAPI handlerRoutine(DWORD dwCtrlType) {
	if(dwCtrlType == CTRL_CLOSE_EVENT || dwCtrlType == CTRL_BREAK_EVENT || dwCtrlType == CTRL_C_EVENT ||
	   dwCtrlType == CTRL_SHUTDOWN_EVENT) {
		CrashHandler::terminate();
		// wait end
		Sleep(5000);
		return TRUE;
	}
	return FALSE;
}

#else /* _WIN32 */

// UNIX

#include <signal.h>

static void onSignal(int sig) {
	CrashHandler::terminate();
}

void CrashHandler::setProcessExceptionHandlers() {
	struct sigaction sa;

	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = &onSignal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
}

void CrashHandler::setThreadExceptionHandlers() {}

#endif
