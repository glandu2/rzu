#include "HttpClientSession.h"
#include "Core/EventLoop.h"
#include "Core/Utils.h"
#include <string.h>

HttpClientSession::HttpClientSession(cval<std::string>& ip, cval<int>& port) : sending(false), ip(ip), port(port) {
	resolver.data = this;
}

void HttpClientSession::sendData(const std::string& url,
                                 const std::string& data,
                                 const std::function<void()>& callback) {
	if(sending) {
		log(LL_Warning, "Already sending %d data\n", (int) dataToSend.size());
	}

	Data request;
	request.data = data;
	request.url = url;
	request.callback = callback;

	dataToSend.push_back(request);

	if(!sending) {
		connectToHost();
		sending = true;
	}
}

void HttpClientSession::connectToHost() {
	struct addrinfo hints;
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = 0;

	currentIp = ip.get();
	currentHost = currentIp;
	currentPort = port.get();

	int result = uv_getaddrinfo(
	    EventLoop::getLoop(), &resolver, &HttpClientSession::onResolved, currentIp.c_str(), nullptr, &hints);
	if(result) {
		log(LL_Error, "Failed to resolve %s: %s(%d), retrying later\n", currentIp.c_str(), uv_strerror(result), result);
		reconnectLater();
	}
}

void HttpClientSession::onResolved(uv_getaddrinfo_t* resolver, int status, struct addrinfo* res) {
	HttpClientSession* thisInstance = (HttpClientSession*) resolver->data;
	if(status < 0) {
		thisInstance->log(LL_Error,
		                  "Failed to resolve %s: %s(%d), retrying later\n",
		                  thisInstance->currentIp.c_str(),
		                  uv_strerror(status),
		                  status);
		thisInstance->reconnectLater();
		return;
	}

	char addr[INET6_ADDRSTRLEN] = {'\0'};
	uv_inet_ntop(AF_INET, &reinterpret_cast<struct sockaddr_in*>(res->ai_addr)->sin_addr, addr, sizeof(addr));

	uv_freeaddrinfo(res);

	thisInstance->log(LL_Info, "Connecting to %s:%d\n", addr, thisInstance->currentPort);
	thisInstance->SocketSession::connect(addr, thisInstance->currentPort);
}

EventChain<SocketSession> HttpClientSession::onConnected() {
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
	                    currentHost.c_str(),
	                    currentPort,
	                    (int) data.data.size());
	write(requestHeader.c_str(), requestHeader.size());
	write(data.data.c_str(), data.data.size());

	log(LL_Debug, "Sending %s\n", requestHeader.c_str());

	return SocketSession::onConnected();
}

EventChain<SocketSession> HttpClientSession::onDataReceived() {
	static const char HTTP_STATUS_OK[] = "HTTP/1.1 200 OK\r\n";
	std::vector<char> data;
	getStream()->readAll(&data);

	std::string str;
	str.assign(data.data(), data.size());
	log(LL_Debug, "Received: %s\n", str.c_str());

	if(strncmp(str.c_str(), HTTP_STATUS_OK, sizeof(HTTP_STATUS_OK) - 1) == 0) {
		const Data& data = dataToSend.front();
		data.callback();
		dataToSend.pop_front();
		log(LL_Info, "Data sent\n");
	} else {
		log(LL_Error, "HTTP response failed: %s\n", str.c_str());
	}

	closeSession();

	return SocketSession::onDataReceived();
}

EventChain<SocketSession> HttpClientSession::onDisconnected(bool causedByRemote) {
	if(dataToSend.empty()) {
		sending = false;
	} else {
		if(causedByRemote) {
			reconnectLater();
		} else {
			connectToHost();
		}
	}
	return SocketSession::onDisconnected(causedByRemote);
}

void HttpClientSession::reconnectLater() {
	reconnectTimer.start(this, &HttpClientSession::connectToHost, 5000, 0);
}
