#ifndef ADMINSERVER_ADMININTERFACE_H
#define ADMINSERVER_ADMININTERFACE_H

#include "Core/Object.h"
#include "NetSession/TelnetSession.h"
#include <string>

class AdminInterface : public TelnetSession
{
	DECLARE_CLASS(AdminInterface)
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
	void listObjectsCount();
};

#endif // ADMINSERVER_ADMININTERFACE_H
