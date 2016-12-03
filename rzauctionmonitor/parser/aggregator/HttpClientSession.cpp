#include "HttpClientSession.h"
#include "Core/Utils.h"

HttpClientSession::HttpClientSession(cval<std::string>& ip, cval<int>& port, cval<std::string>& hostname)
    : hostname(hostname), sending(false), ip(ip), port(port)
{
}

void HttpClientSession::sendData(std::string url, const std::string& data)
{
	if(sending) {
		log(LL_Warning, "Already sending %d data\n", dataToSend.size());
	}

	Data request;
	request.data = data;
	request.url = url;

	dataToSend.push_back(request);

	if(!sending) {
		connect(ip.get().c_str(), port.get());
		sending = true;
	}
}

EventChain<SocketSession> HttpClientSession::onConnected()
{
	const Data& data = dataToSend.front();

	std::string requestHeader;
	Utils::stringFormat(requestHeader,
	                    "POST %s HTTP/1.1\r\n"
	                    "Host: %s:%d\r\n"
	                    "User-Agent: rzu\r\n"
	                    "Accept: */*\r\n"
	                    "Content-Type: application/json\r\n"
	                    "Content-Length: %d\r\n"
	                    "Connection: close\r\n"
	                    "\r\n",
	                    data.url.c_str(),
	                    hostname.get().c_str(),
	                    port.get(),
	                    data.data.size());
	write(requestHeader.c_str(), requestHeader.size());
	write(data.data.c_str(), data.data.size());

	log(LL_Debug, "Sending %s\n", requestHeader.c_str());

	dataToSend.pop_front();

	return SocketSession::onConnected();
}

EventChain<SocketSession> HttpClientSession::onDataReceived()
{
	std::vector<char> data;
	getStream()->readAll(&data);

	std::string str;
	str.assign(data.data(), data.size());
	log(LL_Info, "Received: %s\n", str.c_str());

	closeSession();

	return SocketSession::onDataReceived();
}

EventChain<SocketSession> HttpClientSession::onDisconnected(bool causedByRemote)
{
	if(dataToSend.empty()) {
		sending = false;
	} else {
		if(causedByRemote) {
			reconnectTimer.start(this, &HttpClientSession::reconnect, 1000, 0);
		} else {
			reconnect();
		}
	}
	return SocketSession::onDisconnected(causedByRemote);
}

void HttpClientSession::reconnect()
{
	connect(ip.get().c_str(), port.get());
}
