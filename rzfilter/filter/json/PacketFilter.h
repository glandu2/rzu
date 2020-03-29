#ifndef PACKETFILTER_H
#define PACKETFILTER_H

#include "GameClient/TS_CS_CHAT_REQUEST.h"
#include "GameClient/TS_SC_ATTACK_EVENT.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "IFilter.h"
#include "LibGlobal.h"
#include "Packet/PacketStructsName.h"
#include <unordered_map>

struct TS_SC_CHAT;

class PacketFilter : public IFilter {
public:
	PacketFilter(IFilterEndpoint* client, IFilterEndpoint* server, ServerType serverType, PacketFilter* data);
	~PacketFilter();

	virtual bool onServerPacket(const TS_MESSAGE* packet);
	virtual bool onClientPacket(const TS_MESSAGE* packet);

protected:
private:
	template<class Packet> void showPacketJson(const Packet* packet, int version);

	void printPacketJson(SessionType sessionType, const TS_MESSAGE* packet, int version, bool isServerMsg);

	template<class Packet> struct PrintPacketFunctor {
		bool operator()(PacketFilter* filter, const TS_MESSAGE* packet, int version);
	};

private:
	struct Item {
		ar_handle_t handle;
		uint32_t code;
		uint64_t uid;
		uint64_t count;
	};
	struct Data {};
	Data* data;
};

extern "C" {
SYMBOL_EXPORT IFilter* createFilter(IFilterEndpoint* client,
                                    IFilterEndpoint* server,
                                    IFilter::ServerType serverType,
                                    IFilter* oldFilter);
SYMBOL_EXPORT void destroyFilter(IFilter* filter);
}

#endif  // PACKETFILTER_H
