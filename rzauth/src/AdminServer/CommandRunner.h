#ifndef ADMINSERVER_COMMANDRUNNER_H
#define ADMINSERVER_COMMANDRUNNER_H

#include "Object.h"
#include <string>

namespace AdminServer {

class AdminInterface;

class CommandRunner : public Object
{
	DECLARE_CLASS(AdminServer::CommandRunner)
public:
	CommandRunner(AdminInterface *iface) : iface(iface) {}

	void startServer(const std::string& name);
	void stopServer(const std::string& name);
	void setEnv(const std::string& variableName, const std::string& value);
	void getEnv(const std::string& variableName);
	void closeDbConnections();
	void listGameServers();
	void listObjectsCount();

private:
	AdminInterface *iface;
};

} // namespace AdminServer

#endif // ADMINSERVER_COMMANDRUNNER_H
