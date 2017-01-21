#include "PacketFilter.h"
#include "GameClient/TS_SC_SKILL.h"
#include "GameClient/TS_SC_CHAT.h"
#include "GameClient/TS_SC_ENTER.h"
#include "GameClient/TS_SC_STATE_RESULT.h"
#include "AuthClient/TS_AC_SERVER_LIST.h"
#include <sstream>
#include "Packet/JSONWriter.h"

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

void PacketFilter::sendChatMessage(IFilterEndpoint* client, const char* msg) {
	TS_SC_CHAT* chatRqst;
	size_t msgLen = strlen(msg);
	if(msgLen > 126)
		msgLen = 126;

	chatRqst = TS_MESSAGE_WNA::create<TS_SC_CHAT, char>((int)msgLen+1);

	chatRqst->len = (uint16_t)msgLen;
	strncpy(chatRqst->message, msg, chatRqst->len);
	chatRqst->message[chatRqst->len] = 0;
	strcpy(chatRqst->szSender, "Filter");
	chatRqst->type = 3;

	client->sendPacket(chatRqst);

	TS_MESSAGE_WNA::destroy(chatRqst);
}

bool PacketFilter::onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet) {
	clientp = client;

	switch(packet->id) {
		case TS_SC_SKILL::packetID:
			//packet->process(this,&PacketFilter::showPacketJson<TS_SC_SKILL>, server->getPacketVersion());
			break;

		case TS_SC_STATE_RESULT::packetID: {
			/*const TS_SC_STATE_RESULT* stateResult = reinterpret_cast<const TS_SC_STATE_RESULT*>(packet);
			char buffer[1024];

			sprintf(buffer, "DOT(caster 0x%08X, target 0x%08X, id %d Lv%d, result %d, value %d, targetval %d, total %d, final %d)",
					stateResult->caster_handle,
					stateResult->target_handle,
					stateResult->code,
					stateResult->level,
					stateResult->result_type,
					stateResult->value,
					stateResult->target_value,
					stateResult->total_amount,
					stateResult->final);
			sendChatMessage(client, buffer);
			*/
			break;
		}

		case TS_AC_SERVER_LIST::packetID:
			packet->process(this, &PacketFilter::showPacketJson<TS_AC_SERVER_LIST>, server->getPacketVersion());
			break;

		case TS_SC_INVENTORY::packetID:
			packet->process(this, &PacketFilter::showPacketJson<TS_SC_INVENTORY>, server->getPacketVersion());
			break;

		case TS_SC_ATTACK_EVENT::packetID:
			//packet->process(this, &PacketFilter::showPacketJson<TS_SC_ATTACK_EVENT>, server->getPacketVersion());
			break;

		case_packet_is(TS_SC_ENTER)
		    packet->process(this, &PacketFilter::showPacketJson<TS_SC_ENTER>, server->getPacketVersion());
			break;
	}

	return true;
}

bool PacketFilter::onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet) {
	return true;
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
