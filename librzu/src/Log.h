#ifndef LOG_H
#define LOG_H

#include <string>
#include <vector>
#include "Object.h"
#include "uv.h"
#include "ConfigInfo.h"

struct TS_MESSAGE;

class RAPPELZLIB_EXTERN Log : public Object, public IListener
{
	DECLARE_CLASS(Log)
public:
	enum Level {
		LL_Fatal,
		LL_Error,
		LL_Warning,
		LL_Info,
		LL_Debug,
		LL_Trace
	};

	Log(cval<bool>& enabled, cval<std::string>& fileMaxLevel, cval<std::string>& consoleMaxLevel, cval<std::string>& dir, cval<std::string>& fileName);
	Log(cval<bool>& enabled, Level fileMaxLevel, Level consoleMaxLevel, cval<std::string>& dir, cval<std::string>& fileName);

	~Log();

	void startWriter();
	void stopWriter();
	bool open();
	void close();

	uv_thread_t getLogWriterThread() { return logWritterThreadId; }

	void log(Level level, const char* objectName, size_t objectNameSize, const char* message, ...);
	void log(Level level, const char* objectName, size_t objectNameSize, const char* message, va_list args);

	static Log* get() { return defaultLogger; }
	static Log* setDefaultLogger(Log* newLogger) { Log* old = defaultLogger; defaultLogger = newLogger; return old; }

	Level getMaxLevel() { return fileMaxLevel > consoleMaxLevel ? fileMaxLevel : consoleMaxLevel; }

	bool isAllMessageWritten();

protected:
	static void updateEnabled(IListener* instance, cval<bool>* level);
	static void updateFileLevel(IListener* instance, cval<std::string>* level);
	static void updateConsoleLevel(IListener* instance, cval<std::string>* level);
	static void updateFile(IListener* instance, cval<std::string>* str);

	static void logWritterThreadStatic(void* arg);
	void logWritterThread();

	void updateLevel(bool isConsole, const std::string& level);

private:
	struct Message {
		time_t time;
		std::string objectName;
		std::string message;
		Level level;
		bool writeToConsole;
		bool writeToFile;
	};

	void construct(cval<bool>& enabled, cval<std::string>& dir, cval<std::string>& fileName);

	static Log* defaultLogger;

	uv_thread_t logWritterThreadId;
	uv_mutex_t messageListMutex;
	uv_cond_t messageListCond;
	volatile bool stop;
	std::vector<Message*> messageQueue;

	Level fileMaxLevel;
	Level consoleMaxLevel;
	cval<std::string>& dir;
	cval<std::string>& fileName;
	void* file;
	uv_mutex_t fileMutex;
	uv_timer_t flushTimer;
};

#endif // LOG_H
