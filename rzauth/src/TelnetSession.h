#ifndef ADMINSERVER_TELNETSESSION_H
#define ADMINSERVER_TELNETSESSION_H

#include "SocketSession.h"
#include <string>
#include <sstream>

class TelnetSession : public SocketSession
{
	DECLARE_CLASS(TelnetSession)
public:
	TelnetSession() {}
	virtual ~TelnetSession() {}

	virtual void onConnected() {}
	virtual void onCommand(const std::vector<std::string>& args) = 0;

protected:
	virtual void write(const void* data, int size) { if(getSocket()->getState() == Socket::ConnectedState) getSocket()->write(data, size); }
	virtual void close() { getSocket()->abort(); }

private:
	void onDataReceived();

	void parseData(const std::vector<char>& data);
	void parseCommand(const std::string& data);

private:
	std::vector<char> buffer;
};

#endif // ADMINSERVER_TELNETSESSION_H
