#define __STDC_LIMIT_MACROS
#include "ClientSession.h"
#include <string.h>

#include "Packets/PacketEnums.h"
#include "Packets/TS_SC_RESULT.h"

namespace GameServer {

void ClientSession::init() {
}

void ClientSession::deinit() {
}

ClientSession::ClientSession() {
}

ClientSession::~ClientSession() {
}

void ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_CS_ACCOUNT_WITH_AUTH::packetID:
			onAccountWithAuth(static_cast<const TS_CS_ACCOUNT_WITH_AUTH*>(packet));
			break;
	}
}

void ClientSession::onAccountWithAuth(const TS_CS_ACCOUNT_WITH_AUTH* packet) {
	TS_SC_RESULT result;

	TS_MESSAGE::initMessage(&result);

	result.request_msg_id = packet->id;
	result.result = 0;
	result.value = 0;

	sendPacket(&result);
}

} //namespace UploadServer
