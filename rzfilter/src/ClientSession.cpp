#include "ClientSession.h"
#include "Core/PrintfFormats.h"
#include "GlobalConfig.h"
#include "NetSession/BanManager.h"
#include "Packet/PacketStructsName.h"
#include <algorithm>
#include <string.h>

#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "Packet/PacketEpics.h"
#include "PacketEnums.h"

ClientSession::ClientSession(bool authMode,
                             GameClientSessionManager* gameClientSessionManager,
                             FilterManager* filterManager,
                             FilterManager* converterFilterManager)
    : EncryptedSession<PacketSession>(authMode ? SessionType::AuthClient : SessionType::GameClient,
                                      SessionPacketOrigin::Server,
                                      CONFIG_GET()->client.epic.get()),
      serverSession(new ServerSession(authMode, this, gameClientSessionManager, filterManager, converterFilterManager)),
      authMode(authMode) {}

StreamAddress ClientSession::getAddress() {
	if(getStream())
		return getStream()->getRemoteAddress();
	else
		return StreamAddress{};
}

void ClientSession::banAddress(StreamAddress address) {
	if(getServer() && getServer()->getBanManager())
		getServer()->getBanManager()->banIp(address);
}

bool ClientSession::isStrictForwardEnabled() {
	return CONFIG_GET()->client.strictforward.get();
}

ClientSession::~ClientSession() {}

EventChain<SocketSession> ClientSession::onConnected() {
	log(LL_Info, "Client connected, connecting server session\n");

	if(serverSession)
		serverSession->connect(getServerIp(), getServerPort());
	setDirtyObjectName();
	getStream()->setNoDelay(true);

	return PacketSession::onConnected();
}

EventChain<SocketSession> ClientSession::onDisconnected(bool causedByRemote) {
	log(LL_Info, "Client disconnected, disconnecting server session\n");
	if(serverSession) {
		serverSession->detachClient();
		serverSession->onClientDisconnected();
	}

	return PacketSession::onDisconnected(causedByRemote);
}

void ClientSession::detachServer() {
	serverSession = nullptr;
}

EventChain<PacketSession> ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	if(serverSession)
		serverSession->onClientPacketReceived(packet);

	return PacketSession::onPacketReceived(packet);
}

void ClientSession::updateObjectName() {
	size_t streamNameSize;
	const char* streamName = getStream()->getObjectName(&streamNameSize);
	setObjectName(8 + (int) streamNameSize, "Client[%s]", streamName);
}
