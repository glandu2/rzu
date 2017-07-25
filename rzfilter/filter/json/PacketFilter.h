#ifndef PACKETFILTER_H
#define PACKETFILTER_H

#include "IFilter.h"
#include "LibGlobal.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "GameClient/TS_SC_ATTACK_EVENT.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"
#include "GameClient/TS_CS_CHAT_REQUEST.h"
#include <unordered_map>

struct TS_SC_CHAT;

class PacketFilter : public IFilter
{
public:
	PacketFilter(PacketFilter* data);
	~PacketFilter();

	void sendChatMessage(IFilterEndpoint* client, const char* msg, const char* sender = "Filter", TS_CHAT_TYPE type = CHAT_WHISPER);

	virtual bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType);
	virtual bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType);

protected:
private:
	template<class Packet> void showPacketJson(const Packet* packet, int version);
	void onChatMessage(const TS_SC_CHAT* packet);

	void printAuthPacketJson(const TS_MESSAGE* packet, int version, bool isServerMsg);
	void printGamePacketJson(const TS_MESSAGE* packet, int version, bool isServerMsg);

private:
	struct Item {
		uint32_t handle;
		uint32_t code;
		uint64_t uid;
		uint64_t count;
	};
	struct Data {
		std::unordered_map<uint32_t, Item> items;
	};
	Data* data;

	IFilterEndpoint* clientp;
};

extern "C" {
SYMBOL_EXPORT IFilter* createFilter(IFilter* oldFilter);
SYMBOL_EXPORT void destroyFilter(IFilter* filter);
}

#endif // PACKETFILTER_H
