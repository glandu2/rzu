#ifndef HTTPCLIENTSESSION_H
#define HTTPCLIENTSESSION_H

#include "NetSession/SocketSession.h"
#include "Config/ConfigParamVal.h"
#include <string>
#include <list>
#include "Core/Timer.h"

class HttpClientSession : public SocketSession
{
	DECLARE_CLASSNAME(HttpClientSession, 0)
public:
	HttpClientSession(cval<std::string>& ip, cval<int>& port, cval<std::string>& hostname);

	void sendData(std::string url, const std::string& data);
	size_t getPendingNumber() { return dataToSend.size(); }

protected:
	virtual EventChain<SocketSession> onConnected();
	virtual EventChain<SocketSession> onDataReceived();
	virtual EventChain<SocketSession> onDisconnected(bool causedByRemote);

	void reconnect();

private:
	static void onDataReceivedStatic(IListener* instance, Stream *stream);
	static void onSocketStateChanged(IListener* instance, Stream*, Stream::State oldState, Stream::State newState, bool causedByRemote);
	static void socketError(IListener* instance, Stream* socket, int errnoValue);
private:
	struct Data {
		std::string url;
		std::string data;
	};

	std::list<Data> dataToSend;
	cval<std::string>& hostname;
	bool sending;
	cval<std::string>& ip;
	cval<int>& port;
	Timer<HttpClientSession> reconnectTimer;
};

#endif
