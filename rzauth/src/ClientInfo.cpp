#include "ClientInfo.h"
#include "Network/RappelzSocket.h"
#include "Packets/TS_CA_VERSION.h"
#include "Packets/TS_AC_RESULT.h"
#include "Packets/TS_SC_RESULT.h"
#include <string.h>

ISocket* ClientInfo::serverSocket = new Socket;
std::unordered_map<std::string, ClientInfo*> ClientInfo::clients;

ClientInfo::ClientInfo(RappelzSocket* socket) {
	this->socket = socket;
	this->state = NotLogged;
	this->useRsaAuth = false;

	addInstance(socket->addEventListener(this, &onStateChanged));
	addInstance(socket->addPacketListener(TS_CA_VERSION::packetID, this, &onDataReceived));
}

ClientInfo::~ClientInfo() {
	invalidateCallbacks();
	if(account.size() > 0)
		clients.erase(account);

	socket->deleteLater();
}

void ClientInfo::startServer() {
	serverSocket->addConnectionListener(nullptr, &onNewConnection);
	serverSocket->listen("0.0.0.0", 4500);
}

void ClientInfo::onNewConnection(void* instance, ISocket* serverSocket) {
	static RappelzSocket *newSocket = new RappelzSocket;
	static ClientInfo* clientInfo = new ClientInfo(newSocket);

	do {

		if(!serverSocket->accept(newSocket))
			break;

		printf("new socket2\n");
		newSocket = new RappelzSocket;
		clientInfo = new ClientInfo(newSocket);
	} while(1);
}

void ClientInfo::onStateChanged(void* instance, ISocket* clientSocket, ISocket::State oldState, ISocket::State newState) {
	ClientInfo* thisInstance = static_cast<ClientInfo*>(instance);
	
	if(newState == ISocket::UnconnectedState) {
		delete thisInstance;
	}
}

void ClientInfo::onDataReceived(void* instance, RappelzSocket* clientSocket, const TS_MESSAGE* packet) {
	ClientInfo* thisInstance = static_cast<ClientInfo*>(instance);

	if(packet->msg_check_sum != TS_MESSAGE::checkMessage(packet)) {
		printf("Bad packet\n");
		clientSocket->abort();
		return;
	}

	switch(packet->id) {
		case TS_CA_VERSION::packetID: {
			const TS_CA_VERSION* packetData = static_cast<const TS_CA_VERSION*>(packet);
			if(!strcmp(packetData->szVersion, "TEST")) {
				TS_SC_RESULT result;
				TS_MESSAGE::initMessage<TS_SC_RESULT>(&result);
				result.value = clients.size() ^ 0xADADADAD;
				result.result = 0;
				result.request_msg_id = packetData->id;
				clientSocket->sendPacket(&result);
			}
		}
	}
}
