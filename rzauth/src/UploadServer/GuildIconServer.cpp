#include "GuildIconServer.h"
#include "EventLoop.h"
#include "../GlobalConfig.h"
#include <string.h>
#include <stdio.h>

namespace UploadServer {

GuildIconServer::GuildIconServer(Socket* socket) {
	this->socket = socket;
	this->retrievingUrl = true;

	socket->addDataListener(this, &onDataReceived);
}

void GuildIconServer::startServer() {
	Socket* serverSocket = new Socket(EventLoop::getLoop());
	srand((unsigned int)time(NULL));
	serverSocket->addConnectionListener(nullptr, &onNewConnection);
	serverSocket->listen(CONFIG_GET()->upload.client.listenIp,
						 CONFIG_GET()->upload.client.webPort);
}

GuildIconServer::~GuildIconServer() {
	socket->deleteLater();
}

void GuildIconServer::onNewConnection(ICallbackGuard* instance, Socket* serverSocket) {
	static Socket *newSocket = new Socket(EventLoop::getLoop());
	static GuildIconServer* serverInfo = new GuildIconServer(newSocket);

	do {
		if(!serverSocket->accept(newSocket))
			break;

		newSocket = new Socket(EventLoop::getLoop());
		serverInfo = new GuildIconServer(newSocket);
	} while(1);
}

void GuildIconServer::onStateChanged(ICallbackGuard* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState) {
	GuildIconServer* thisInstance = static_cast<GuildIconServer*>(instance);

	if(newState == Socket::UnconnectedState) {
		delete thisInstance;
	}
}

void GuildIconServer::onDataReceived(ICallbackGuard *instance, Socket* socket) {
	GuildIconServer* thisInstance = static_cast<GuildIconServer*>(instance);
	char buffer[1024];
	buffer[1023] = 0;

	while(socket->getAvailableBytes() > 0) {
		size_t nbread = socket->read(buffer, 1023);

		if(thisInstance->retrievingUrl) {
			char *p = strpbrk(buffer, "\r\n");
			if(p) {
				thisInstance->retrievingUrl = false;
				thisInstance->url.write(buffer, p - buffer);
			} else {
				thisInstance->url.write(buffer, nbread);
			}
		}

		if(strstr(buffer, "\r\n\r\n")) {
			thisInstance->parseUrl(thisInstance->url.str());
			thisInstance->url.str(std::string());
			thisInstance->url.clear();
			thisInstance->retrievingUrl = true;

		}
		//discard when not reading url
	}
}


void GuildIconServer::parseUrl(std::string urlString) {
	if(urlString.size() < 14 || (urlString.size() == 14 && urlString.at(4) == '/')) {
		//Minimum number of char to have a correct http get request, also exclude GET / HTTP/1.1
		socket->abort();
		return;
	}
	size_t beforeUrl = 4;
	size_t afterUrl = urlString.size() - 9;

	trace("Received request\n");

	if(strncmp(urlString.c_str(), "GET ", 4)) {
		trace("Not a get method\n");
		socket->abort();
		return;
	} else if(urlString.compare(afterUrl, std::string::npos, " HTTP/1.1")) {
		trace("Not a HTTP/1.1 request\n");
		socket->abort();
		return;
	}

	size_t p;
	for(p = afterUrl-1; p > beforeUrl; p--) {
		if(urlString.at(p) == '/' || urlString.at(p) == '\\')
			break;
	}
	sendIcon(urlString.substr(p+1, afterUrl-p-1));
}

void GuildIconServer::sendIcon(const std::string& filename) {
	const char * const noFoundData =
			"HTTP/1.1 404 Not Found\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 22\r\n"
			"\r\n"
			"<h1>404 Not Found</h1>";

	const char * const foundData =
			"HTTP/1.1 200 Ok\r\n"
			"Content-Type: image/jpeg\r\n"
			"Content-Length: %ld\r\n"
			"\r\n";

	std::string fullFileName = CONFIG_GET()->upload.client.uploadDir.get() + "/" + filename;
	FILE* file = fopen(fullFileName.c_str(), "rb");
	if(!file) {
		socket->write(noFoundData, strlen(noFoundData));
	} else {
		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);
		if(fileSize > 64000)
			fileSize = 64000;

		int bufferSize = strlen(foundData) + 10 + fileSize;
		char *buffer = new char[bufferSize];
		size_t fileContentBegin = sprintf(buffer, foundData, (long int)fileSize);

		size_t bytesTransferred = 0;
		size_t nbrw;
		while(bytesTransferred < fileSize) {
			nbrw = fread(buffer + fileContentBegin + bytesTransferred, 1, fileSize - bytesTransferred, file);
			if(nbrw <= 0)
				break;
			bytesTransferred += nbrw;
		}

		if(nbrw > 0) {
			socket->write(buffer, fileContentBegin + fileSize);
		} else {
			socket->write(noFoundData, strlen(noFoundData));
		}
	}
}

} //namespace UploadServer
