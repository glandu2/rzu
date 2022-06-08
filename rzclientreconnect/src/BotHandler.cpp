#include "BotHandler.h"
#include "ConnectionToClient.h"
#include "Core/Utils.h"
#include "MultiServerManager.h"
#include "Packet/PacketBaseMessage.h"

#include "GameClient/TS_SC_CHAT.h"
#include <charconv>

BotHandler::BotHandler(MultiServerManager* multiServerManager, ConnectionToClient* connectionToClient)
    : multiServerManager(multiServerManager), connectionToClient(connectionToClient) {}

void BotHandler::onPacketFromClient(const TS_MESSAGE* packet) {
	return;
}

void BotHandler::onPacketFromServer(ConnectionToServer* originatingServer, const TS_MESSAGE* packet) {
	packet_type_id_t packetType = PacketMetadata::convertPacketIdToTypeId(
	    packet->id, SessionType::GameClient, SessionPacketOrigin::Server, connectionToClient->getPacketVersion());
	switch(packetType) {
		case TS_SC_CHAT::packetID:
			packet->process(this, &BotHandler::onChat, connectionToClient->getPacketVersion(), originatingServer);
			break;
	}
}
void BotHandler::onChat(const TS_SC_CHAT* packet, ConnectionToServer* originatingServer) {
	if(packet->type == CHAT_PARTY_SYSTEM || packet->type == CHAT_RAID_SYSTEM) {
		auto args = Utils::stringSplit(packet->message, '|');

		if(args[0] == "INVITE") {
			if(!multiServerManager->isKnownLocalPlayer(args[1])) {
				log(LL_Info, "Received party invitation from unknown player %s, ignoring\n", args[1].c_str());
				return;
			}

			int32_t partyId;
			int32_t password;

			if(std::from_chars(args[3].c_str(), args[3].c_str() + args[3].size(), partyId).ec != std::errc{}) {
				log(LL_Error, "Received party invitation with invalid number for partyId: %s\n", args[3].c_str());
				return;
			}

			if(std::from_chars(args[4].c_str(), args[4].c_str() + args[4].size(), password).ec != std::errc{}) {
				log(LL_Error, "Received party invitation with invalid number for password: %s\n", args[4].c_str());
				return;
			}

			// Autoaccept invitation
			log(LL_Info, "Received party invitation from %s, autoaccepting\n", args[1].c_str());
			TS_CS_CHAT_REQUEST partyAcceptPacket{};
			Utils::stringFormat(partyAcceptPacket.message, "/pjoin %d %d", partyId, password);
			originatingServer->sendPacket(partyAcceptPacket);
		}
	}
}