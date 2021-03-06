#pragma once

#include "../Extern.h"
#include "uv.h"
#include <string>
#include <vector>

class IWritableConsole;
template<class T> class cval;
class IListener;

class RZU_EXTERN CrashHandler {
public:
	typedef void (*TerminateCallback)(void* instance);

public:
	static void init();
	static void deinit();
	// Sets exception handlers that work on per-process basis
	static void setProcessExceptionHandlers();

	// Installs C++ exception handlers that function on per-thread basis
	static void setThreadExceptionHandlers();

	static void setDumpMode(IListener*, cval<int>*);

	static void setTerminateCallback(TerminateCallback callback, void* instance);

	static void terminate();
	static void commandTerminate(IWritableConsole*, const std::vector<std::string>&);
	static void commandListObjectsCount(IWritableConsole* console, const std::vector<std::string>&);

private:
	static void onTerminate(uv_async_t*);
	static uv_async_t asyncCallback;
	static void* callbackInstance;
	static bool interruptAttemptInProgress;
	static bool globalHandlersInitialized;
};

