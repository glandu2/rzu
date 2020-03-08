#ifndef HTTPCLIENTSESSION_H
#define HTTPCLIENTSESSION_H

#include "Config/ConfigParamVal.h"
#include "Core/Timer.h"
#include "NetSession/SocketSession.h"
#include <functional>
#include <list>
#include <string>

class HttpClientSession : public SocketSession {
	DECLARE_CLASSNAME(HttpClientSession, 0)

public:
	HttpClientSession(cval<std::string>& ip, cval<int>& port);

	void sendData(const std::string& url, const std::string& data, const std::function<void()>& callback);
	size_t getPendingNumber() { return dataToSend.size(); }

protected:
	virtual EventChain<SocketSession> onConnected();
	virtual EventChain<SocketSession> onDataReceived();
	virtual EventChain<SocketSession> onDisconnected(bool causedByRemote);

	static void onResolved(uv_getaddrinfo_t* resolver, int status, struct addrinfo* res);

	void connectToHost();
	void reconnectLater();

private:
	static void onDataReceivedStatic(IListener* instance, Stream* stream);
	static void onSocketStateChanged(
	    IListener* instance, Stream*, Stream::State oldState, Stream::State newState, bool causedByRemote);
	static void socketError(IListener* instance, Stream* socket, int errnoValue);

private:
	struct Data {
		std::string url;
		std::string data;
		std::function<void()> callback;
	};

	std::list<Data> dataToSend;
	// cval<std::string>& hostname;
	bool sending;
	cval<std::string>& ip;
	cval<int>& port;
	Timer<HttpClientSession> reconnectTimer;
	uv_getaddrinfo_t resolver;

	std::string currentIp;
	std::string currentHost;
	int currentPort;
};

#endif
