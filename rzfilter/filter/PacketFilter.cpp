#include "PacketFilter.h"
#include <sstream>
#include "Packet/JSONWriter.h"
#include "Core/Utils.h"

#include "GameClient/TS_CS_ACCOUNT_WITH_AUTH.h"
#include "GameClient/TS_CS_AUCTION_BID.h"
#include "GameClient/TS_CS_AUCTION_BIDDED_LIST.h"
#include "GameClient/TS_CS_AUCTION_CANCEL.h"
#include "GameClient/TS_CS_AUCTION_INSTANT_PURCHASE.h"
#include "GameClient/TS_CS_AUCTION_REGISTER.h"
#include "GameClient/TS_CS_AUCTION_SEARCH.h"
#include "GameClient/TS_CS_AUCTION_SELLING_LIST.h"
#include "GameClient/TS_CS_BUY_FROM_BOOTH.h"
#include "GameClient/TS_CS_CANCEL_ACTION.h"
#include "GameClient/TS_CS_CHANGE_LOCATION.h"
#include "GameClient/TS_CS_CHARACTER_LIST.h"
#include "GameClient/TS_CS_CHAT_REQUEST.h"
#include "GameClient/TS_CS_CHECK_BOOTH_STARTABLE.h"
#include "GameClient/TS_CS_CHECK_CHARACTER_NAME.h"
#include "GameClient/TS_CS_CONTACT.h"
#include "GameClient/TS_CS_CREATE_CHARACTER.h"
#include "GameClient/TS_CS_DELETE_CHARACTER.h"
#include "GameClient/TS_CS_DIALOG.h"
#include "GameClient/TS_CS_ENTER_EVENT_AREA.h"
#include "GameClient/TS_CS_GAME_TIME.h"
#include "GameClient/TS_CS_GET_BOOTHS_NAME.h"
#include "GameClient/TS_CS_ITEM_KEEPING_LIST.h"
#include "GameClient/TS_CS_ITEM_KEEPING_TAKE.h"
#include "GameClient/TS_CS_LEAVE_EVENT_AREA.h"
#include "GameClient/TS_CS_LOGIN.h"
#include "GameClient/TS_CS_LOGOUT.h"
#include "GameClient/TS_CS_MOVE_REQUEST.h"
#include "GameClient/TS_CS_PUTOFF_ITEM.h"
#include "GameClient/TS_CS_PUTON_ITEM.h"
#include "GameClient/TS_CS_QUERY.h"
#include "GameClient/TS_CS_REGION_UPDATE.h"
#include "GameClient/TS_CS_REPORT.h"
#include "GameClient/TS_CS_REQUEST_LOGOUT.h"
#include "GameClient/TS_CS_RETURN_LOBBY.h"
#include "GameClient/TS_CS_SELL_TO_BOOTH.h"
#include "GameClient/TS_CS_START_BOOTH.h"
#include "GameClient/TS_CS_STOP_BOOTH.h"
#include "GameClient/TS_CS_STOP_WATCH_BOOTH.h"
#include "GameClient/TS_CS_TARGETING.h"
#include "GameClient/TS_CS_UPDATE.h"
#include "GameClient/TS_CS_USE_ITEM.h"
#include "GameClient/TS_CS_VERSION.h"
#include "GameClient/TS_CS_WATCH_BOOTH.h"
#include "GameClient/TS_SC_ADDED_SKILL_LIST.h"
#include "GameClient/TS_SC_ATTACK_EVENT.h"
#include "GameClient/TS_SC_AUCTION_BIDDED_LIST.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"
#include "GameClient/TS_SC_AUCTION_SELLING_LIST.h"
#include "GameClient/TS_SC_BELT_SLOT_INFO.h"
#include "GameClient/TS_SC_BOOTH_CLOSED.h"
#include "GameClient/TS_SC_BOOTH_TRADE_INFO.h"
#include "GameClient/TS_SC_CHANGE_LOCATION.h"
#include "GameClient/TS_SC_CHANGE_NAME.h"
#include "GameClient/TS_SC_CHARACTER_LIST.h"
#include "GameClient/TS_SC_CHAT.h"
#include "GameClient/TS_SC_CHAT_LOCAL.h"
#include "GameClient/TS_SC_CHAT_RESULT.h"
#include "GameClient/TS_SC_COMMERCIAL_STORAGE_INFO.h"
#include "GameClient/TS_SC_DETECT_RANGE_UPDATE.h"
#include "GameClient/TS_SC_DIALOG.h"
#include "GameClient/TS_SC_DISCONNECT_DESC.h"
#include "GameClient/TS_SC_ENTER.h"
#include "GameClient/TS_EQUIP_SUMMON.h"
#include "GameClient/TS_SC_EXP_UPDATE.h"
#include "GameClient/TS_SC_GAME_TIME.h"
#include "GameClient/TS_SC_GET_BOOTHS_NAME.h"
#include "GameClient/TS_SC_GOLD_UPDATE.h"
#include "GameClient/TS_SC_HPMP.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "GameClient/TS_SC_ITEM_KEEPING_LIST.h"
#include "GameClient/TS_SC_ITEM_WEAR_INFO.h"
#include "GameClient/TS_SC_LEAVE.h"
#include "GameClient/TS_SC_LEVEL_UPDATE.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_SC_MOVE.h"
#include "GameClient/TS_SC_MOVE_ACK.h"
#include "GameClient/TS_SC_PROPERTY.h"
#include "GameClient/TS_SC_QUEST_LIST.h"
#include "GameClient/TS_SC_REGEN_HPMP.h"
#include "GameClient/TS_SC_REGION_ACK.h"
#include "GameClient/TS_SC_RESULT.h"
#include "GameClient/TS_SC_SET_TIME.h"
#include "GameClient/TS_SC_SKILL.h"
#include "GameClient/TS_SC_STATE.h"
#include "GameClient/TS_SC_STATE_RESULT.h"
#include "GameClient/TS_SC_STATUS_CHANGE.h"
#include "GameClient/TS_SC_STAT_INFO.h"
#include "GameClient/TS_SC_URL_LIST.h"
#include "GameClient/TS_SC_WARP.h"
#include "GameClient/TS_SC_WATCH_BOOTH.h"
#include "GameClient/TS_SC_WEAR_INFO.h"
#include "GameClient/TS_SC_WEATHER_INFO.h"
#include "GameClient/TS_TIMESYNC.h"

#define PACKET_TO_JSON(type_) \
	case type_::packetID: \
	sizeof(&type_::getSize); \
	packet->process(this, &PacketFilter::showPacketJson<type_>, version); \
	break;

#define PACKET_TO_JSON_2(type_) \
	case_packet_is(type_) \
	packet->process(this, &PacketFilter::showPacketJson<type_>, version); \
	break;

PacketFilter::PacketFilter(PacketFilter *oldFilter)
{
	if(oldFilter) {
		data = oldFilter->data;
		oldFilter->data = nullptr;
	} else {
		data = new Data;
	}
}

PacketFilter::~PacketFilter()
{
	if(data)
		delete data;
}

void PacketFilter::sendChatMessage(IFilterEndpoint* client, const char* msg, const char* sender, TS_CHAT_TYPE type) {
	TS_SC_CHAT chatRqst;

	chatRqst.message = msg;
	if(chatRqst.message.size() > 32766)
		chatRqst.message.resize(32766);
	chatRqst.message.append(1, '\0');

	chatRqst.szSender = sender;
	chatRqst.type = type;

	client->sendPacket(chatRqst);
}

bool PacketFilter::onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet) {
	clientp = client;

	printPacketJson(packet, server->getPacketVersion());

	switch(packet->id) {
//		case TS_SC_STATE_RESULT::packetID: {
//			TS_SC_STATE_RESULT stateResult;
//			bool ok = packet->process(stateResult, EPIC_LATEST);
//			char buffer[1024];

//			sprintf(buffer, "DOT(caster 0x%08X, target 0x%08X, id %d Lv%d, result %d, value %d, targetval %d, total %d, final %d)",
//					stateResult.caster_handle,
//					stateResult.target_handle,
//					stateResult.code,
//					stateResult.level,
//					stateResult.result_type,
//					stateResult.value,
//					stateResult.target_value,
//					stateResult.total_amount,
//					stateResult.final);
//			sendChatMessage(client, buffer);

//			break;
//		}

		case TS_SC_CHAT::packetID: {
			TS_SC_CHAT chatPacket;
			if(packet->process(chatPacket, server->getPacketVersion())) {
				onChatMessage(&chatPacket);
				return false;
			}
		}
	}

	return true;
}

bool PacketFilter::onClientPacket(IFilterEndpoint*, IFilterEndpoint* server, const TS_MESSAGE* packet) {
	printPacketJson(packet, server->getPacketVersion());
	return true;
}

void PacketFilter::printPacketJson(const TS_MESSAGE* packet, int version) {
	switch(packet->id) {
		PACKET_TO_JSON(TS_CS_ACCOUNT_WITH_AUTH);
		PACKET_TO_JSON(TS_CS_AUCTION_BID);
		PACKET_TO_JSON(TS_CS_AUCTION_BIDDED_LIST);
		PACKET_TO_JSON(TS_CS_AUCTION_CANCEL);
		PACKET_TO_JSON(TS_CS_AUCTION_INSTANT_PURCHASE);
		PACKET_TO_JSON(TS_CS_AUCTION_REGISTER);
		PACKET_TO_JSON(TS_CS_AUCTION_SEARCH);
		PACKET_TO_JSON(TS_CS_AUCTION_SELLING_LIST);
		PACKET_TO_JSON(TS_CS_BUY_FROM_BOOTH);
		PACKET_TO_JSON(TS_CS_CANCEL_ACTION);
		PACKET_TO_JSON(TS_CS_CHANGE_LOCATION);
		PACKET_TO_JSON_2(TS_CS_CHARACTER_LIST);
		PACKET_TO_JSON(TS_CS_CHAT_REQUEST);
		PACKET_TO_JSON(TS_CS_CHECK_BOOTH_STARTABLE);
		PACKET_TO_JSON(TS_CS_CHECK_CHARACTER_NAME);
		PACKET_TO_JSON(TS_CS_CONTACT);
		PACKET_TO_JSON(TS_CS_CREATE_CHARACTER);
		PACKET_TO_JSON(TS_CS_DELETE_CHARACTER);
		PACKET_TO_JSON(TS_CS_DIALOG);
		PACKET_TO_JSON(TS_CS_ENTER_EVENT_AREA);
		PACKET_TO_JSON(TS_CS_GAME_TIME);
		PACKET_TO_JSON(TS_CS_GET_BOOTHS_NAME);
		PACKET_TO_JSON(TS_CS_ITEM_KEEPING_LIST);
		PACKET_TO_JSON(TS_CS_ITEM_KEEPING_TAKE);
		PACKET_TO_JSON(TS_CS_LEAVE_EVENT_AREA);
		PACKET_TO_JSON_2(TS_CS_LOGIN);
		PACKET_TO_JSON(TS_CS_LOGOUT);
		PACKET_TO_JSON_2(TS_CS_MOVE_REQUEST);
		PACKET_TO_JSON(TS_CS_PUTOFF_ITEM);
		PACKET_TO_JSON(TS_CS_PUTON_ITEM);
		PACKET_TO_JSON(TS_CS_QUERY);
		PACKET_TO_JSON_2(TS_CS_REGION_UPDATE);
		PACKET_TO_JSON(TS_CS_REPORT);
		PACKET_TO_JSON(TS_CS_REQUEST_LOGOUT);
		PACKET_TO_JSON(TS_CS_RETURN_LOBBY);
		PACKET_TO_JSON(TS_CS_SELL_TO_BOOTH);
		PACKET_TO_JSON(TS_CS_START_BOOTH);
		PACKET_TO_JSON(TS_CS_STOP_BOOTH);
		PACKET_TO_JSON(TS_CS_STOP_WATCH_BOOTH);
		PACKET_TO_JSON(TS_CS_TARGETING);
		PACKET_TO_JSON(TS_CS_UPDATE);
		PACKET_TO_JSON(TS_CS_USE_ITEM);
		PACKET_TO_JSON_2(TS_CS_VERSION);
		PACKET_TO_JSON(TS_CS_WATCH_BOOTH);
		PACKET_TO_JSON(TS_SC_ADDED_SKILL_LIST);
		PACKET_TO_JSON(TS_SC_ATTACK_EVENT);
		PACKET_TO_JSON(TS_SC_AUCTION_BIDDED_LIST);
		PACKET_TO_JSON(TS_SC_AUCTION_SEARCH);
		PACKET_TO_JSON(TS_SC_AUCTION_SELLING_LIST);
		PACKET_TO_JSON(TS_SC_BELT_SLOT_INFO);
		PACKET_TO_JSON(TS_SC_BOOTH_CLOSED);
		PACKET_TO_JSON(TS_SC_BOOTH_TRADE_INFO);
		PACKET_TO_JSON(TS_SC_CHANGE_LOCATION);
		PACKET_TO_JSON(TS_SC_CHANGE_NAME);
		PACKET_TO_JSON(TS_SC_CHARACTER_LIST);
		PACKET_TO_JSON(TS_SC_CHAT);
		PACKET_TO_JSON(TS_SC_CHAT_LOCAL);
		PACKET_TO_JSON(TS_SC_CHAT_RESULT);
		PACKET_TO_JSON(TS_SC_COMMERCIAL_STORAGE_INFO);
		PACKET_TO_JSON(TS_SC_DETECT_RANGE_UPDATE);
		PACKET_TO_JSON(TS_SC_DIALOG);
		PACKET_TO_JSON(TS_SC_DISCONNECT_DESC);
		PACKET_TO_JSON_2(TS_SC_ENTER);
		PACKET_TO_JSON(TS_EQUIP_SUMMON);
		PACKET_TO_JSON(TS_SC_EXP_UPDATE);
		PACKET_TO_JSON(TS_SC_GAME_TIME);
		PACKET_TO_JSON(TS_SC_GET_BOOTHS_NAME);
		PACKET_TO_JSON(TS_SC_GOLD_UPDATE);
		PACKET_TO_JSON(TS_SC_HPMP);
		PACKET_TO_JSON(TS_SC_INVENTORY);
		PACKET_TO_JSON(TS_SC_ITEM_KEEPING_LIST);
		PACKET_TO_JSON(TS_SC_ITEM_WEAR_INFO);
		PACKET_TO_JSON(TS_SC_LEAVE);
		PACKET_TO_JSON(TS_SC_LEVEL_UPDATE);
		PACKET_TO_JSON_2(TS_SC_LOGIN_RESULT);
		PACKET_TO_JSON(TS_SC_MOVE);
		PACKET_TO_JSON(TS_SC_MOVE_ACK);
		PACKET_TO_JSON(TS_SC_PROPERTY);
		PACKET_TO_JSON(TS_SC_QUEST_LIST);
		PACKET_TO_JSON(TS_SC_REGEN_HPMP);
		PACKET_TO_JSON(TS_SC_REGION_ACK);
		PACKET_TO_JSON(TS_SC_RESULT);
		PACKET_TO_JSON(TS_SC_SET_TIME);
		PACKET_TO_JSON(TS_SC_SKILL);
		PACKET_TO_JSON(TS_SC_STATE);
		PACKET_TO_JSON(TS_SC_STATE_RESULT);
		PACKET_TO_JSON(TS_SC_STATUS_CHANGE);
		PACKET_TO_JSON(TS_SC_STAT_INFO);
		PACKET_TO_JSON(TS_SC_URL_LIST);
		PACKET_TO_JSON(TS_SC_WARP);
		PACKET_TO_JSON(TS_SC_WATCH_BOOTH);
		PACKET_TO_JSON(TS_SC_WEAR_INFO);
		PACKET_TO_JSON(TS_SC_WEATHER_INFO);
		PACKET_TO_JSON(TS_TIMESYNC);

		default:
			Object::logStatic(Object::LL_Warning, "rzfilter_module", "packet id %d unknown\n", packet->id);
			break;
	}
}

void PacketFilter::onChatMessage(const TS_SC_CHAT* packet)
{
	//9:38:23
	//2017-01-29 17:26:22
	struct tm currentTime;
	char newMessage[32767];
	Utils::getGmTime(time(nullptr), &currentTime);

	if(packet->szSender[0] == '@') {
		snprintf(newMessage, sizeof(newMessage), "<b>%02d:%02d:%02d</b>: Next message: %s",
		         currentTime.tm_hour,
		         currentTime.tm_min,
		         currentTime.tm_sec,
		         packet->szSender.c_str());
		newMessage[sizeof(newMessage)-1] = '\0';
		sendChatMessage(clientp, newMessage);
		clientp->sendPacket(*packet);
		return;
	}

	snprintf(newMessage, sizeof(newMessage), "<b>%02d:%02d:%02d</b>: %s",
	         currentTime.tm_hour,
	         currentTime.tm_min,
	         currentTime.tm_sec,
	         packet->message.c_str());
	newMessage[sizeof(newMessage)-1] = '\0';

	sendChatMessage(clientp, newMessage, packet->szSender.c_str(), packet->type);
}

IFilter *createFilter(IFilter *oldFilter)
{
	Object::logStatic(Object::LL_Info, "rzfilter_module", "Loaded filter from data: %p\n", oldFilter);
	return new PacketFilter((PacketFilter*)oldFilter);
}

void destroyFilter(IFilter *filter)
{
	delete filter;
}

template<class Packet>
void PacketFilter::showPacketJson(const Packet* packet)
{
	JSONWriter jsonWriter(EPIC_LATEST, false);
	packet->serialize(&jsonWriter);
	jsonWriter.finalize();
	std::string jsonData = jsonWriter.toString();

	Object::logStatic(Object::LL_Info, "rzfilter_module", "%s packet:\n%s\n", typeid(Packet).name(), jsonData.c_str());
}
