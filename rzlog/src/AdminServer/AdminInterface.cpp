#include "AdminInterface.h"
#include "NetSession/ServersManager.h"
#include "Config/ConfigInfo.h"
#include <stdlib.h>
#include "ClassCounter.h"
#include "Database/DbConnectionPool.h"
#include "Core/CrashHandler.h"
#include <string.h>
#include "Core/Utils.h"

static const char MSG_WELCOME[] = "Log server - Administration server - Type \"help\" for a list of available commands\r\n> ";

static const char MSG_UNKNOWN_COMMAND[] = "Unknown command\r\n";
static const char MSG_HELP[] =
		"Available commands:\r\n"
		"- start <server_name>     Start the server <server_name>. Servers names are listed when starting. If <server_name> is \"all\", all server with autostart on are started.\r\n"
		"- stop <server_name>      Same as start <server_name> but stop the server. \"stop all\" will stop all servers and exit.\r\n"
		"- set <variable> <value>  Set config variable <variable> to <value>. Double-quotes are allowed for values with spaces. Use \"\" for a escaped double quote character.\r\n"
		"- get <variable>          Get config variable value.\r\n"
		"- mem                     List object counts.\r\n"
		"- closedb                 Close all idle database connections (use this to bring a database offline).\r\n"
		"\r\n";


#ifdef __GLIBC__
#include <malloc.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <crtdbg.h>
#include <malloc.h>
#endif

namespace AdminServer {

static const char MSG_UNKNOWN_SERVER_NAME[] = "Unknown server name\r\n";
static const char MSG_START_OK[] = "Server started\r\n";
static const char MSG_STOP_OK[] = "Server stopped\r\n";
static const char MSG_UNKNOWN_VARIABLE[] = "Unknown variable name\r\n";

AdminInterface::AdminInterface() {
	info("New admin session\n");
}

AdminInterface::~AdminInterface() {
	info("Admin session closed\n");
}

void AdminInterface::onConnected() {
	write(MSG_WELCOME, sizeof(MSG_WELCOME));
}

void AdminInterface::onCommand(const std::vector<std::string> &args) {
	if(args.size() > 1 && args[0] == "start")
		startServer(args[1]);
	else if(args.size() > 1 && args[0] == "stop")
		stopServer(args[1]);
	else if(args.size() > 2 && args[0] == "set")
		setEnv(args[1], args[2]);
	else if(args.size() > 1 && args[0] == "get")
		getEnv(args[1]);
	else if(args.size() > 0 && args[0] == "mem")
		listObjectsCount();
	else if(args.size() > 0 && args[0] == "closedb")
		closeDbConnections();
	else if(args.size() > 0 && args[0] == "help")
		write(MSG_HELP, sizeof(MSG_HELP));
	else if(args.size() > 0 && args[0] == "terminate")
		CrashHandler::terminate();
	else
		write(MSG_UNKNOWN_COMMAND, sizeof(MSG_UNKNOWN_COMMAND));
	write("> ", 2);
}

void AdminInterface::startServer(const std::string& name) {
	bool ok;

	if(!strcmp(name.c_str(), "all"))
		ok = ServersManager::getInstance()->start();
	else
		ok = ServersManager::getInstance()->start(name);

	if(!ok) {
		write(MSG_UNKNOWN_SERVER_NAME, sizeof(MSG_UNKNOWN_SERVER_NAME));
	} else {
		info("Server %s started via admin session\n", name.c_str());
		write(MSG_START_OK, sizeof(MSG_START_OK));
	}
}

void AdminInterface::stopServer(const std::string& name) {
	bool ok;

	if(!strcmp(name.c_str(), "all"))
		ok = ServersManager::getInstance()->stop();
	else
		ok = ServersManager::getInstance()->stop(name);

	if(!ok) {
		write(MSG_UNKNOWN_SERVER_NAME, sizeof(MSG_UNKNOWN_SERVER_NAME));
	} else {
		info("Server %s stopped via admin session\n", name.c_str());
		write(MSG_STOP_OK, sizeof(MSG_STOP_OK));
	}
}

void AdminInterface::setEnv(const std::string& variableName, const std::string& value) {
	ConfigValue* v = ConfigInfo::get()->getValue(variableName);

	if(v == nullptr) {
		write(MSG_UNKNOWN_VARIABLE, sizeof(MSG_UNKNOWN_VARIABLE));
		return;
	}

	switch(v->getType()) {
		case ConfigValue::Bool:
			v->setBool(!strcmp(value.c_str(), "true") || !strcmp(value.c_str(), "1"));
			break;

		case ConfigValue::Integer:
			v->setInt(atoi(value.c_str()));
			break;

		case ConfigValue::Float:
			v->setFloat((float)atof(value.c_str()));
			break;

		case ConfigValue::String:
			v->setString(value);
			break;
	}

	getEnv(variableName);
}

void AdminInterface::getEnv(const std::string& variableName) {
	ConfigValue* v = ConfigInfo::get()->getValue(variableName);

	if(v == nullptr) {
		write(MSG_UNKNOWN_VARIABLE, sizeof(MSG_UNKNOWN_VARIABLE));
		return;
	}

	std::string val;
	char type = 'U';

	switch(v->getType()) {
		case ConfigValue::Bool:
			type = 'B';
			val = v->getBool() ? "true" : "false";
			break;

		case ConfigValue::Integer:
			type = 'N';
			val = Utils::convertToString(v->getInt());
			break;

		case ConfigValue::Float:
			type = 'F';
			val = Utils::convertToString(v->getFloat());
			break;

		case ConfigValue::String:
			type = 'S';
			val = v->getString();
			break;
	}

	char buffer[1024];
	int len = sprintf(buffer, "%c%c%s:%s\r\n", type, v->isDefault() ? '*' : ' ', variableName.c_str(), val.c_str());

	write(buffer, len);
}

void AdminInterface::closeDbConnections() {
	int connectionsClosed = DbConnectionPool::getInstance()->closeAllConnections();
	char buffer[1024];
	int len = sprintf(buffer, "Closed %d DB connections\r\n", connectionsClosed);
	write(buffer, len);
}

void AdminInterface::listObjectsCount() {
	std::map<std::string, unsigned long*>::const_iterator it, itEnd;
	char buffer[1024];
	int len;

#ifdef __GLIBC__
	struct mallinfo memUsage = mallinfo();
	len = sprintf(buffer,
				  "Memory usage:\r\n"
				  " heap size: %d\r\n"
				  " unused chunks: %d\r\n"
				  " mmap chunks: %d\r\n"
				  " mmap mem size: %d\r\n"
				  " used mem size: %d\r\n"
				  " unused mem size: %d\r\n"
				  " trailing releasable size: %d\r\n\r\n",
				  memUsage.arena,
				  memUsage.ordblks,
				  memUsage.hblks,
				  memUsage.hblkhd,
				  memUsage.uordblks,
				  memUsage.fordblks,
				  memUsage.keepcost);
	write(buffer, len);
#endif

#if defined(_WIN32) && defined(_DEBUG)
	_CrtMemState memUsage;
    size_t heapSize = 0, heapCommit = 0;
    memset(&memUsage, 0, sizeof(memUsage));
	_CrtMemDumpStatistics(&memUsage);
    _heapused(&heapSize, &heapCommit);
	len = sprintf(buffer,
				  "Memory usage:\r\n"
                  " heap size: %ld\r\n"
                  " commit size: %ld\r\n"
                  " normal block size: %ld\r\n"
                  " free block size: %ld\r\n"
                  " CRT block size: %ld\r\n"
                  " client block size: %ld\r\n"
                  " ignore block size: %ld\r\n"
                  " peak memory size: %ld\r\n"
                  " used memory size: %ld\r\n\r\n",
                  heapSize,
                  heapCommit,
                  memUsage.lSizes[_NORMAL_BLOCK],
                  memUsage.lSizes[_FREE_BLOCK],
                  memUsage.lSizes[_CRT_BLOCK],
                  memUsage.lSizes[_CLIENT_BLOCK],
                  memUsage.lSizes[_IGNORE_BLOCK],
                  memUsage.lHighWaterCount,
                  memUsage.lTotalCount);
	write(buffer, len);
#endif

	for(it = getObjectsCount().cbegin(), itEnd = getObjectsCount().cend(); it != itEnd; ++it) {
		len = sprintf(buffer, "%s: %ld\r\n",
					  it->first.c_str(),
					  *it->second);
		write(buffer, len);
	}
}

} // namespace AdminServer
