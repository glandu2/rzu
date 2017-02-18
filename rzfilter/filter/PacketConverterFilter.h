#ifndef PACKETCONVERTERFILTER_H
#define PACKETCONVERTERFILTER_H

#include "IFilter.h"
#include "LibGlobal.h"

struct TS_SC_CHAT;

class PacketConverterFilter : public IFilter
{
public:
	PacketConverterFilter(PacketConverterFilter* data);
	~PacketConverterFilter();

	virtual bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet);
	virtual bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet);

protected:
	bool convertPacketAndSend(IFilterEndpoint* target, const TS_MESSAGE* packet, int version, bool isServerMsg);
};

extern "C" {
SYMBOL_EXPORT IFilter* createFilter(IFilter* oldFilter);
SYMBOL_EXPORT void destroyFilter(IFilter* filter);
}

#endif // PACKETCONVERTERFILTER_H
