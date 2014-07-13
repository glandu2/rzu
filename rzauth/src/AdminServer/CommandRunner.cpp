#include "CommandRunner.h"
#include "ServersManager.h"
#include "AdminInterface.h"
#include "ConfigInfo.h"
#include <stdlib.h>
#include "../AuthServer/GameServerSession.h"
#include "ClassCounter.h"


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

void CommandRunner::startServer(const std::string& name) {
	bool ok;

	if(!strcmp(name.c_str(), "all"))
		ok = ServersManager::getInstance()->start();
	else
		ok = ServersManager::getInstance()->start(name);

	if(!ok) {
		iface->write(MSG_UNKNOWN_SERVER_NAME, sizeof(MSG_UNKNOWN_SERVER_NAME));
	} else {
		info("Server %s started via admin session\n", name.c_str());
		iface->write(MSG_START_OK, sizeof(MSG_START_OK));
	}
}

void CommandRunner::stopServer(const std::string& name) {
	bool ok;

	if(!strcmp(name.c_str(), "all"))
		ok = ServersManager::getInstance()->stop();
	else
		ok = ServersManager::getInstance()->stop(name);

	if(!ok) {
		iface->write(MSG_UNKNOWN_SERVER_NAME, sizeof(MSG_UNKNOWN_SERVER_NAME));
	} else {
		info("Server %s stopped via admin session\n", name.c_str());
		iface->write(MSG_STOP_OK, sizeof(MSG_STOP_OK));
	}
}

void CommandRunner::setEnv(const std::string& variableName, const std::string& value) {
	ConfigValue* v = ConfigInfo::get()->getValue(variableName);

	if(v == nullptr) {
		iface->write(MSG_UNKNOWN_VARIABLE, sizeof(MSG_UNKNOWN_VARIABLE));
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

#ifdef _MSC_VER
#define INT2STR(i) std::to_string((long long)(i))
#define FLOAT2STR(i) std::to_string((long double)(i))
#else
#define INT2STR(i) std::to_string(i)
#define FLOAT2STR(i) std::to_string(i)
#endif

void CommandRunner::getEnv(const std::string& variableName) {
	ConfigValue* v = ConfigInfo::get()->getValue(variableName);

	if(v == nullptr) {
		iface->write(MSG_UNKNOWN_VARIABLE, sizeof(MSG_UNKNOWN_VARIABLE));
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
			val = INT2STR(v->getInt());
			break;

		case ConfigValue::Float:
			type = 'F';
			val = FLOAT2STR(v->getFloat());
			break;

		case ConfigValue::String:
			type = 'S';
			val = v->getString();
			break;
	}

	char buffer[1024];
	int len = sprintf(buffer, "%c%c%s:%s\r\n", type, v->isDefault() ? '*' : ' ', variableName.c_str(), val.c_str());

	iface->write(buffer, len);
}

void CommandRunner::listGameServers() {
	const std::unordered_map<uint16_t, AuthServer::GameServerSession*>& serverList = AuthServer::GameServerSession::getServerList();
	std::unordered_map<uint16_t, AuthServer::GameServerSession*>::const_iterator it, itEnd;

	char buffer[1024];
	int len;

	len = sprintf(buffer, "%u gameserver(s)\r\n", (unsigned int)serverList.size());
	iface->write(buffer, len);

	for(it = serverList.cbegin(), itEnd = serverList.cend(); it != itEnd; ++it) {
		AuthServer::GameServerSession* server = it->second;

		len = sprintf(buffer, "Index: %2d, name: %10s, address: %s:%d, players count: %u, screenshot url: %s\r\n",
					  server->getServerIdx(),
					  server->getServerName().c_str(),
					  server->getServerIp().c_str(),
					  server->getServerPort(),
					  server->getPlayerCount(),
					  server->getServerScreenshotUrl().c_str());
		iface->write(buffer, len);
	}
}

void CommandRunner::listObjectsCount() {
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
	iface->write(buffer, len);
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
	iface->write(buffer, len);
#endif

	for(it = getObjectsCount().cbegin(), itEnd = getObjectsCount().cend(); it != itEnd; ++it) {
		len = sprintf(buffer, "%s: %ld\r\n",
					  it->first.c_str(),
					  *it->second);
		iface->write(buffer, len);
	}

}

} // namespace AdminServer
