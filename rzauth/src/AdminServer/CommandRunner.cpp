#include "CommandRunner.h"
#include "../ServersManager.h"
#include "AdminInterface.h"
#include "ConfigInfo.h"
#include <stdlib.h>

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

} // namespace AdminServer
