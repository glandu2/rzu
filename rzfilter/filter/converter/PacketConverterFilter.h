#ifndef PACKETCONVERTERFILTER_H
#define PACKETCONVERTERFILTER_H

#include "IFilter.h"
#include "LibGlobal.h"
#include "SpecificPacketConverter.h"

class PacketConverterFilter : public IFilter
{
public:
	PacketConverterFilter(PacketConverterFilter* data);
	~PacketConverterFilter();

	virtual bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType);
	virtual bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType);

protected:
	bool convertAuthPacketAndSend(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, bool isServerMsg);
	bool convertGamePacketAndSend(IFilterEndpoint* target, const TS_MESSAGE* packet, int version, bool isServerMsg);

private:
	SpecificPacketConverter specificPacketConverter;
};

extern "C" {
SYMBOL_EXPORT IFilter* createFilter(IFilter* oldFilter);
SYMBOL_EXPORT void destroyFilter(IFilter* filter);
}

#endif // PACKETCONVERTERFILTER_H
