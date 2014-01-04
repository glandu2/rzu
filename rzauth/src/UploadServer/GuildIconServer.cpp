#include "GuildIconServer.h"
#include "EventLoop.h"
#include "../GlobalConfig.h"
#include <string.h>
#include <stdio.h>
#include "Utils.h"

namespace UploadServer {

static const char * const htmlNotFound =
		"HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 22\r\n"
		"\r\n"
		"<h1>404 Not Found</h1>";
static int htmlNotFoundSize = strlen(htmlNotFound);

static const char * const htmlFound =
		"HTTP/1.1 200 Ok\r\n"
		"Content-Type: image/jpeg\r\n"
		"Content-Length: %ld\r\n"
		"\r\n";
static int htmlFoundSize = strlen(htmlFound);

GuildIconServer::GuildIconServer(Socket* socket) {
	this->socket = socket;
	this->status = WaitStatusLine;
	this->nextByteToMatch = 0;
	this->urlLength = 0;

	socket->addDataListener(this, &onDataReceived);
}

void GuildIconServer::startServer() {
	Socket* serverSocket = new Socket(EventLoop::getLoop());
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
	std::vector<char> buffer;

	while(socket->getAvailableBytes() > 0) {
		socket->readAll(&buffer);
		thisInstance->parseData(buffer);
	}
}

void GuildIconServer::parseData(const std::vector<char>& data) {
	static const char * const beginUrl = "GET ";
	static const char * const endHeader = "\r\n\r\n";
	ssize_t size = data.size();
	const char* begin = &data[0];

	for(const char* p = begin; p - begin < size; p++) {
		if(status == WaitStatusLine) {
			if(*p == beginUrl[nextByteToMatch]) {
				nextByteToMatch++;
				if(nextByteToMatch >= 4) {
					status = RetrievingStatusLine;
					nextByteToMatch = 0;
				}
			} else {
				nextByteToMatch = 0;
			}
		} else if(status == RetrievingStatusLine) {
			if(*p == '\r' || *p == '\n') {
				status = WaitEndOfHeaders;
			} else if(*p) {
				if(urlLength < 255) {
					url.put(*p);
					urlLength++;
				} else {
					status = WaitStatusLine;
					nextByteToMatch = 0;
					url.str(std::string());
					url.clear();
					urlLength = 0;
				}
			}
		}

		if(status == RetrievingStatusLine || status == WaitEndOfHeaders) {
			if(*p == endHeader[nextByteToMatch]) {
				nextByteToMatch++;
				if(nextByteToMatch >= 4) {
					status = WaitStatusLine;
					nextByteToMatch = 0;

					std::string urlString = url.str();
					if(!urlString.compare(urlString.size() - 9, std::string::npos, " HTTP/1.1")) {
						urlString.resize(urlString.size() - 9);
						parseUrl(urlString);
					}

					url.str(std::string());
					url.clear();
					urlLength = 0;
				}
			} else {
				nextByteToMatch = 0;
			}
		}
	}
}


void GuildIconServer::parseUrl(std::string urlString) {
	size_t p;
	for(p = urlString.size()-1; p >= 0; p--) {
		if(urlString.at(p) == '/' || urlString.at(p) == '\\')
			break;
	}
	if(p+1 >= urlString.size()) {
		//attempt to get a directory
		socket->write(htmlNotFound, htmlNotFoundSize);
	} else {
		sendIcon(urlString.substr(p+1, std::string::npos));
	}
}

void GuildIconServer::sendIcon(const std::string& filename) {
	std::string fullFileName = Utils::getFullPath(CONFIG_GET()->upload.client.uploadDir.get() + "/" + filename);
	FILE* file = fopen(fullFileName.c_str(), "rb");

	if(!file) {
		socket->write(htmlNotFound, htmlNotFoundSize);
	} else {
		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);
		if(fileSize > 64000)
			fileSize = 64000;

		int bufferSize = htmlFoundSize + 10 + fileSize;
		char *buffer = new char[bufferSize];
		size_t fileContentBegin = sprintf(buffer, htmlFound, (long int)fileSize);

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
			socket->write(htmlNotFound, htmlNotFoundSize);
		}
	}
}

} //namespace UploadServer
