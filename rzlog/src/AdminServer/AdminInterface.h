#ifndef ADMINSERVER_ADMININTERFACE_H
#define ADMINSERVER_ADMININTERFACE_H

#include "Object.h"
#include "TelnetSession.h"
#include <string>

namespace AdminServer {

class AdminInterface : public TelnetSession
{
	DECLARE_CLASS(AdminServer::AdminInterface)
public:
	AdminInterface();
	virtual ~AdminInterface();

protected:
	void onConnected();
	void onCommand(const std::vector<std::string>& args);

	void startServer(const std::string& name);
	void stopServer(const std::string& name);
	void setEnv(const std::string& variableName, const std::string& value);
	void getEnv(const std::string& variableName);
	void closeDbConnections();
	void listObjectsCount();
};

} // namespace AdminServer

#endif // ADMINSERVER_ADMININTERFACE_H
