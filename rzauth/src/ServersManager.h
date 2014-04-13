#ifndef SERVERSMANAGER_H
#define SERVERSMANAGER_H

#include "Object.h"
#include <unordered_map>
#include "RappelzServer.h"
#include <string>
#include "BanManager.h"


class ConfigValue;

class ServersManager : public Object
{
public:
	ServersManager();
	~ServersManager();

	void stop();
	void start();

	RappelzServerCommon* getServer(const std::string& name);

	static ServersManager* getInstance() { return instance; }

protected:

	void addServer(const char* name, RappelzServerCommon* server, ConfigValue& listenIp, ConfigValue& listenPort, BanManager* banManager = nullptr);

private:
	struct ServerInfo {
		ServerInfo(RappelzServerCommon* server, ConfigValue* listenIp, ConfigValue* listenPort, BanManager* banManager = nullptr) :
			server(server), listenIp(listenIp), listenPort(listenPort), banManager(banManager) {}

		RappelzServerCommon* server;
		ConfigValue* listenIp;
		ConfigValue* listenPort;
		BanManager* banManager;
	};
	std::unordered_map<std::string, ServerInfo*> servers;
	BanManager banManager;

	static ServersManager* instance;
};

#endif // SERVERSMANAGER_H
