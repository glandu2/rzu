#pragma once

#include "SocketSession.h"
#include <string>

class RZU_EXTERN TelnetSession : public SocketSession {
	DECLARE_CLASS(TelnetSession)
public:
	TelnetSession() {}
	virtual ~TelnetSession() {}

	virtual void onCommand(const std::vector<std::string>& args) = 0;

protected:
	virtual size_t write(const void* data, size_t size) {
		if(getStream()->getState() == Stream::ConnectedState)
			return SocketSession::write(data, size);
		else
			return 0;
	}
	virtual void close() { abortSession(); }

private:
	EventChain<SocketSession> onDataReceived();

	void parseData(const std::vector<char>& data);
	void parseCommand(const std::string& data);

private:
	std::vector<char> buffer;
};

