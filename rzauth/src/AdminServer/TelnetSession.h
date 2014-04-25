#ifndef ADMINSERVER_TELNETSESSION_H
#define ADMINSERVER_TELNETSESSION_H

#include "../SocketSession.h"
#include <string>
#include <sstream>
#include "AdminInterface.h"

namespace AdminServer {

class CommandRunner;

class TelnetSession : public SocketSession, public AdminInterface
{
	DECLARE_CLASS(AdminServer::TelnetSession)
public:
	TelnetSession();
	~TelnetSession();

	virtual void onConnected();

protected:
	void onDataReceived();

	void parseData(const std::vector<char>& data);
	void parseCommand(const std::string& data);

	virtual void write(const void* data, int size) { getSocket()->write(data, size); }
	virtual void close() { getSocket()->abort(); }

private:
	std::vector<char> buffer;
	CommandRunner* commandRunner;
};

} // namespace AdminServer

#endif // ADMINSERVER_TELNETSESSION_H
