#include "ServerInfo.h"
#include "ClientInfo.h"
#include <string.h>

#include "Packets/PacketEnums.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"
#include "Packets/TS_AG_CLIENT_LOGIN.h"
#include "Packets/TS_AG_KICK_CLIENT.h"

Socket* ServerInfo::serverSocket = new Socket;
std::vector<ServerInfo*> ServerInfo::servers;

ServerInfo::ServerInfo(RappelzSocket* socket)
{
	addInstance(socket->addEventListener(this, &onStateChanged));
	addInstance(socket->addPacketListener(TS_GA_LOGIN::packetID, this, &onDataReceived));
	addInstance(socket->addPacketListener(TS_GA_CLIENT_LOGIN::packetID, this, &onDataReceived));
	addInstance(socket->addPacketListener(TS_GA_CLIENT_LOGOUT::packetID, this, &onDataReceived));
	addInstance(socket->addPacketListener(TS_GA_CLIENT_KICK_FAILED::packetID, this, &onDataReceived));
}

void ServerInfo::startServer() {
	srand(time(NULL));
	serverSocket->addConnectionListener(nullptr, &onNewConnection);
	serverSocket->listen("0.0.0.0", 4502);
}

ServerInfo::~ServerInfo() {
	invalidateCallbacks();

	if(serverIdx >= 0 && (size_t)serverIdx < servers.size())
		servers[serverIdx] = nullptr;

	socket->deleteLater();
}

void ServerInfo::onNewConnection(void* instance, Socket* serverSocket) {
	static RappelzSocket *newSocket = new RappelzSocket;
	static ServerInfo* serverInfo = new ServerInfo(newSocket);

	do {

		if(!serverSocket->accept(newSocket))
			break;

		printf("new socket2\n");
		newSocket = new RappelzSocket;
		serverInfo = new ServerInfo(newSocket);
	} while(1);
}

void ServerInfo::onStateChanged(void* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState) {
	ServerInfo* thisInstance = static_cast<ServerInfo*>(instance);

	if(newState == Socket::UnconnectedState) {
		delete thisInstance;
	}
}

void ServerInfo::onDataReceived(void* instance, RappelzSocket* clientSocket, const TS_MESSAGE* packet) {
	ServerInfo* thisInstance = static_cast<ServerInfo*>(instance);

	if(packet->msg_check_sum != TS_MESSAGE::checkMessage(packet)) {
		printf("Bad packet\n");
		clientSocket->abort();
		return;
	}

	switch(packet->id) {
		case TS_GA_LOGIN::packetID:
			thisInstance->onServerLogin(static_cast<const TS_GA_LOGIN*>(packet));
			break;

		case TS_GA_CLIENT_LOGIN::packetID:
			thisInstance->onClientLogin(static_cast<const TS_GA_CLIENT_LOGIN*>(packet));
			break;

		case TS_GA_CLIENT_LOGOUT::packetID:
			thisInstance->onClientLogout(static_cast<const TS_GA_CLIENT_LOGOUT*>(packet));
			break;

		case TS_GA_CLIENT_KICK_FAILED::packetID:
			thisInstance->onClientKickFailed(static_cast<const TS_GA_CLIENT_KICK_FAILED*>(packet));
			break;
	}
}

void ServerInfo::onServerLogin(const TS_GA_LOGIN* packet) {
	serverIdx = packet->server_idx;
	serverName = packet->server_name;
	serverIp = packet->server_ip;
	serverPort = packet->server_port;
	serverScreenshotUrl = packet->server_screenshot_url;
	isAdultServer = packet->is_adult_server;

	TS_AG_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_AG_LOGIN_RESULT>(&result);

	if(servers.size() <= (size_t)serverIdx)
		servers.resize(serverIdx+1, nullptr);

	if(servers.at(serverIdx) == nullptr) {
		servers[serverIdx] = this;
		result.result = TS_RESULT_SUCCESS;
	} else {
		result.result = TS_RESULT_ALREADY_EXIST;
	}

	socket->sendPacket(&result);
}

void ServerInfo::onClientLogin(const TS_GA_CLIENT_LOGIN* packet) {
	TS_AG_CLIENT_LOGIN result;
	ClientData* client;
	std::unordered_map<std::string, ClientData*>::const_iterator it = pendingClients.find(std::string(packet->account));
	if(it != pendingClients.cend()) {
		client = it->second;
		pendingClients.erase(it);
	} else
		client = nullptr;

	TS_MESSAGE::initMessage<TS_AG_CLIENT_LOGIN>(&result);
	strncpy(result.account, packet->account, 61);

	if(client == nullptr) {
		result.nAccountID = 0;
		result.result = TS_RESULT_ACCESS_DENIED;
		result.nPCBangUser = 0;
		result.nEventCode = 0;
		result.nAge = 0;
		result.nContinuousPlayTime = 0;
		result.nContinuousLogoutTime = 0;
	} else {
		//To complete
		result.nAccountID = client->accountId;
		result.result = TS_RESULT_SUCCESS;
		result.nPCBangUser = 0;
		result.nEventCode = client->eventCode;
		result.nAge = client->age;
		result.nContinuousPlayTime = 0;
		result.nContinuousLogoutTime = 0;

		userCount++;
	}

	socket->sendPacket(&result);
}

void ServerInfo::onClientLogout(const TS_GA_CLIENT_LOGOUT* packet) {
	userCount--;
}

void ServerInfo::onClientKickFailed(const TS_GA_CLIENT_KICK_FAILED* packet) {

}
