#ifndef ADMINSERVER_TELNETSESSION_H
#define ADMINSERVER_TELNETSESSION_H

#include "../SocketSession.h"
#include <string>
#include <sstream>

namespace AdminServer {

class TelnetSession : public SocketSession
{
public:
	TelnetSession();

protected:
	void onDataReceived();

	void parseData(const std::vector<char>& data);
	void parseCommand(const std::string& data);

private:
	std::vector<char> buffer;
};

} // namespace AdminServer

#endif // ADMINSERVER_TELNETSESSION_H
