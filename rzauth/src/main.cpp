#include "EventLoop.h"
#include "GlobalConfig.h"
#include "RappelzLibInit.h"

#include "AuthServer/DB_Account.h"
#include "ServersManager.h"
#include "SocketSession.h"


/* TODO
 * Log packets
 * Telnet for commands (like stop which would help to have a correct valgrind output):
 *  - stop - stop the server
 *  - stats - show stats of the server (player count, active connections, connected GS, ...)
 *  - set - set variable value
 *  - get - get variable value
 *  - dump - dump variables
 */

void runServers();
void showDebug(uv_timer_t*);

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>

//1: no dump, anything else: dump
static long long int dumpMode = 0;

LONG WINAPI crashHandler(EXCEPTION_POINTERS *ExceptionInfo) {
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
	char *szResult = NULL;
	char szDumpPath[_MAX_PATH*2];
	char szScratch [_MAX_PATH*2];


	if(dumpMode != 1) {
		if (GetModuleFileName(NULL, szDumpPath, _MAX_PATH)) {
			char* p = strrchr(szDumpPath, '\\');
			if(p) {
				*(p+1) = 0;
			}
		} else {
			if (!GetTempPath(_MAX_PATH, szDumpPath))
				strcpy(szDumpPath, "c:\\temp\\");
		}

		strcat(szDumpPath, "crashdump.dmp");

		// create the file
		HANDLE hFile = CreateFile(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
								  FILE_ATTRIBUTE_NORMAL, NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION ExInfo;

			ExInfo.ThreadId = GetCurrentThreadId();
			ExInfo.ExceptionPointers = ExceptionInfo;
			ExInfo.ClientPointers = FALSE;

			// write the dump
			BOOL bOK = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, &ExInfo, NULL, NULL);
			if (bOK)
			{
				sprintf(szScratch, "Saved dump file to '%s'", szDumpPath);
				szResult = szScratch;
				retval = EXCEPTION_EXECUTE_HANDLER;
			}
			else
			{
				sprintf(szScratch, "Failed to save dump file to '%s' (error %d)", szDumpPath, GetLastError());
				szResult = szScratch;
			}
			CloseHandle(hFile);
		}
		else
		{
			sprintf(szScratch, "Failed to create dump file '%s' (error %d)", szDumpPath, GetLastError());
			szResult = szScratch;
		}
	}

	if (szResult)
		MessageBox(NULL, szResult, "Application crashed", MB_OK);

	return retval;
}
#endif

int main(int argc, char **argv) {
//	uv_timer_t timer;
//	uv_timer_init(EventLoop::getLoop(), &timer);
//	uv_timer_start(&timer, &showDebug, 0, 3000);

#ifdef _WIN32
	SetUnhandledExceptionFilter(&crashHandler);
#endif

	RappelzLibInit(argc, argv, &GlobalConfig::init);
	if(AuthServer::DB_Account::init() == false) {
		return -1;
	}

#ifdef _WIN32
	dumpMode = CONFIG_GET()->admin.dumpMode;
#endif

	runServers();

	//Make valgrind happy
	EventLoop::getInstance()->deleteObjects();

	return 0;
}

void runServers() {
	ServersManager serverManager;
	serverManager.start();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

void showDebug(uv_timer_t *) {
	char debugInfo[1000];
	strcpy(debugInfo, "----------------------------------\n");
	sprintf(debugInfo, "%s%lu socket Sessions\n", debugInfo, SocketSession::getObjectCount());
	sprintf(debugInfo, "%sstats.connections = %d\n", debugInfo, CONFIG_GET()->stats.connectionCount.get());
	sprintf(debugInfo, "%sstats.disconnections = %d\n", debugInfo, CONFIG_GET()->stats.disconnectionCount.get());
	puts(debugInfo);
}
