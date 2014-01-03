#include "ClientInfo.h"
#include "RappelzSocket.h"
#include "EventLoop.h"
#include "../GlobalConfig.h"
#include "Packets/PacketEnums.h"
#include "GameServerInfo.h"
#include <time.h>
#include <stdio.h>


#ifdef _WIN32
#include <direct.h>
#define createdir(dir) mkdir(dir)
#define snprintf(buffer, size, ...) _snprintf_s(buffer, size, _TRUNCATE, __VA_ARGS__)
#else
#define createdir(dir) mkdir(dir, 0755)
#endif

// From ffmpeg http://www.ffmpeg.org/doxygen/trunk/cutils_8c-source.html
#define ISLEAP(y) (((y) % 4 == 0) && (((y) % 100) != 0 || ((y) % 400) == 0))
#define LEAPS_COUNT(y) ((y)/4 - (y)/100 + (y)/400)

/* This is our own gmtime_r. It differs from its POSIX counterpart in a
	couple of places, though. */
struct tm *brktimegm(time_t secs, struct tm *tm)
{
	int days, y, ny, m;
	int md[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	days = secs / 86400;
	secs %= 86400;
	tm->tm_hour = secs / 3600;
	tm->tm_min = (secs % 3600) / 60;
	tm->tm_sec =  secs % 60;

	/* oh well, may be someone some day will invent a formula for this stuff */
	y = 1970; /* start "guessing" */
	while (days > 365) {
		ny = (y + days/366);
		days -= (ny - y) * 365 + LEAPS_COUNT(ny - 1) - LEAPS_COUNT(y - 1);
		y = ny;
	}
	if (days==365 && !ISLEAP(y)) { days=0; y++; }
	md[1] = ISLEAP(y)?29:28;
	for (m=0; days >= md[m]; m++)
		days -= md[m];

	tm->tm_year = y;  /* unlike gmtime_r we store complete year here */
	tm->tm_mon = m+1; /* unlike gmtime_r tm_mon is from 1 to 12 */
	tm->tm_mday = days+1;

	return tm;
}

#include "Packets/TS_UC_LOGIN_RESULT.h"
#include "Packets/TS_UC_UPLOAD.h"

namespace UploadServer {

ClientInfo::ClientInfo(RappelzSocket* socket) {
	this->socket = socket;
	currentRequest = nullptr;

	socket->addEventListener(this, &onStateChanged);
	socket->addPacketListener(TS_CU_LOGIN::packetID, this, &onDataReceived);
	socket->addPacketListener(TS_CU_UPLOAD::packetID, this, &onDataReceived);
}

ClientInfo::~ClientInfo() {
	socket->deleteLater();
}

void ClientInfo::startServer() {
	Socket* serverSocket = new Socket(EventLoop::getLoop());
	srand((unsigned int)time(NULL));
	serverSocket->addConnectionListener(nullptr, &onNewConnection);
	serverSocket->listen(CONFIG_GET()->upload.client.listenIp,
						 CONFIG_GET()->upload.client.port);
}

void ClientInfo::onNewConnection(ICallbackGuard* instance, Socket* serverSocket) {
	static RappelzSocket *newSocket = new RappelzSocket(EventLoop::getLoop(), true);
	static ClientInfo* clientInfo = new ClientInfo(newSocket);

	do {

		if(!serverSocket->accept(newSocket))
			break;

		newSocket = new RappelzSocket(EventLoop::getLoop(), true);
		clientInfo = new ClientInfo(newSocket);
	} while(1);
}

void ClientInfo::onStateChanged(ICallbackGuard* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState) {
	ClientInfo* thisInstance = static_cast<ClientInfo*>(instance);

	if(newState == Socket::UnconnectedState) {
		delete thisInstance;
	}
}

void ClientInfo::onDataReceived(ICallbackGuard* instance, RappelzSocket*, const TS_MESSAGE* packet) {
	ClientInfo* thisInstance = static_cast<ClientInfo*>(instance);

	switch(packet->id) {
		case TS_CU_LOGIN::packetID:
			thisInstance->onLogin(static_cast<const TS_CU_LOGIN*>(packet));
			break;

		case TS_CU_UPLOAD::packetID:
			thisInstance->onUpload(static_cast<const TS_CU_UPLOAD*>(packet));
			break;
	}
}

void ClientInfo::onLogin(const TS_CU_LOGIN* packet) {
	TS_UC_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_UC_LOGIN_RESULT>(&result);

	debug("Upload from client %s:%d, client id %u with account id %u for guild id %u on server %30s\n",
		  socket->getHost().c_str(),
		  socket->getPort(),
		  packet->client_id,
		  packet->account_id,
		  packet->guild_id,
		  packet->raw_server_name);

	currentRequest = UploadRequest::popRequest(packet->client_id, packet->account_id, packet->guild_id, packet->one_time_password, packet->raw_server_name);

	if(currentRequest)
		result.result = TS_RESULT_SUCCESS;
	else
		result.result = TS_RESULT_INVALID_ARGUMENT;

	socket->sendPacket(&result);
}

void ClientInfo::onUpload(const TS_CU_UPLOAD* packet) {
	TS_UC_UPLOAD result;
	TS_MESSAGE::initMessage<TS_UC_UPLOAD>(&result);


	debug("Upload from client %s:%d\n", socket->getHost().c_str(), socket->getPort());

	if(currentRequest == nullptr) {
		warn("Upload attempt without a request from %s:%d\n", socket->getHost().c_str(), socket->getPort());
		result.result = TS_RESULT_NOT_EXIST;
	} else if(packet->file_length != packet->size - sizeof(TS_CU_UPLOAD)) {
		warn("Upload packet size invalid, received %u bytes but the packet say %u bytes\n", packet->size - sizeof(TS_CU_UPLOAD), packet->file_length);
		result.result = TS_RESULT_INVALID_ARGUMENT;
	} else if(packet->file_length > 64000) {
		debug("Upload file is too large: %d bytes. Max 64000 bytes\n", packet->file_length);
		result.result = TS_RESULT_LIMIT_MAX;
	} else if(!checkJpegImage(packet->file_contents)) {
	} else {
		int filenameSize = currentRequest->getGameServer()->getName().size() + 1 + 10 + 1 + 2 + 2 + 2 + 1 + 2 + 2 + 2 + 4 + 1;
		char *filename = (char*)alloca(filenameSize);
		struct tm timeinfo;

		brktimegm(time(NULL), &timeinfo);

		snprintf(filename, filenameSize, "%s_%010d_%02d%02d%02d_%02d%02d%02d.jpg",
				 currentRequest->getGameServer()->getName().c_str(),
				 currentRequest->getGuildId(),
				 timeinfo.tm_year % 100,
				 timeinfo.tm_mon,
				 timeinfo.tm_mday,
				 timeinfo.tm_hour,
				 timeinfo.tm_min,
				 timeinfo.tm_sec);

		debug("Uploading image %s for client id %u with account id %u for guild %u\n", filename, currentRequest->getClientId(), currentRequest->getAccountId(), currentRequest->getGuildId());

		std::string fullFileName = CONFIG_GET()->upload.client.uploadDir.get() + "/" + filename;
		createdir(CONFIG_GET()->upload.client.uploadDir.get().c_str());
		FILE* file = fopen(fullFileName.c_str(), "wb");
		if(!file) {
			warn("Cant open upload target file %s\n", fullFileName.c_str());
			result.result = TS_RESULT_ACCESS_DENIED;
		} else {
			int dataWritten = fwrite(&packet->file_contents, packet->file_length, 1, file);
			fclose(file);

			if(dataWritten == 1) {
				result.result = TS_RESULT_SUCCESS;
				currentRequest->getGameServer()->sendUploadResult(currentRequest->getGuildId(), packet->file_length, filename);
			} else {
				warn("Cant write upload target file %s, disk full ?\n", fullFileName.c_str());
				result.result = TS_RESULT_ACCESS_DENIED;
			}
		}
	}

	socket->sendPacket(&result);
}

bool ClientInfo::checkJpegImage(const char *data) {
	return true;
}

} // namespace UploadServer
