#include "ClientGameSession.h"
#include "ClientAuthSession.h"
#include "GameClient/TS_CS_VERSION.h"
#include "GameClient/TS_CS_ACCOUNT_WITH_AUTH.h"
#include "GameClient/TS_SC_RESULT.h"

ClientGameSession::ClientGameSession()
{
}

void ClientGameSession::onConnected() {
	TS_CS_VERSION versionMsg;
	TS_CS_ACCOUNT_WITH_AUTH loginInGameServerMsg;

	//continue server move as we are connected now to game server
	TS_MESSAGE::initMessage<TS_CS_VERSION>(&versionMsg);
	strcpy(versionMsg.szVersion, auth->getVersion().c_str());
	sendPacket(&versionMsg);

	strcpy(loginInGameServerMsg.account, auth->getAccountName().c_str());
	loginInGameServerMsg.one_time_key = auth->getOnTimePassword();
	sendPacket(loginInGameServerMsg, EPIC_9_1);
}

void ClientGameSession::onDisconnected(bool causedByRemote) {
	onGameDisconnected();
	auth->onGameDisconnected();
}

void ClientGameSession::onPacketReceived(const TS_MESSAGE *packet) {
	switch(packet->id) {
		case TS_SC_RESULT::packetID: {
			const TS_SC_RESULT* resultMsg = reinterpret_cast<const TS_SC_RESULT*>(packet);
			if(resultMsg->request_msg_id == TS_CS_ACCOUNT_WITH_AUTH::packetID) {
				auth->onGameResult((TS_ResultCode)resultMsg->result);
				if(resultMsg->result == TS_RESULT_SUCCESS)
					onGameConnected();
			} else {
				onGamePacketReceived(packet);
			}
			break;
		}

		default:
			onGamePacketReceived(packet);
	}
}