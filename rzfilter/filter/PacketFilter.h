#ifndef PACKETFILTER_H
#define PACKETFILTER_H

#include "IFilter.h"
#include "LibGlobal.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "GameClient/TS_SC_ATTACK_EVENT.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"
#include <unordered_map>

struct TS_SC_ENTER;
struct TS_SC_SKILL;

class PacketFilter : public IFilter
{
public:
	PacketFilter(PacketFilter* data);
	~PacketFilter();

	void sendChatMessage(IFilterEndpoint* client, const char* msg);

	virtual bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet);
	virtual bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet);

private:
	void onInventory(const TS_SC_INVENTORY *packet);
	void onAttack(const TS_SC_ATTACK_EVENT *packet);
	void onEnter(const TS_SC_ENTER *packet);
	void onSkill(const TS_SC_SKILL *packet);

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
