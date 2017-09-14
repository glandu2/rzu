#ifndef PACKETCONVERTERFILTER_H
#define PACKETCONVERTERFILTER_H

#include "IFilter.h"
#include "LibGlobal.h"
#include "SpecificPacketConverter.h"

class PacketConverterFilter : public IFilter
{
public:
	PacketConverterFilter(IFilterEndpoint* client, IFilterEndpoint* server, PacketConverterFilter* data);
	~PacketConverterFilter();

	virtual bool onServerPacket(const TS_MESSAGE* packet, ServerType serverType);
	virtual bool onClientPacket(const TS_MESSAGE* packet, ServerType serverType);

protected:
	bool convertAuthPacketAndSend(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, bool isServerMsg);
	bool convertGamePacketAndSend(IFilterEndpoint* target, const TS_MESSAGE* packet, int version, bool isServerMsg);

private:
	SpecificPacketConverter specificPacketConverter;
};

extern "C" {
SYMBOL_EXPORT IFilter* createFilter(IFilterEndpoint* client, IFilterEndpoint* server, IFilter* oldFilter);
SYMBOL_EXPORT void destroyFilter(IFilter* filter);
}

#endif // PACKETCONVERTERFILTER_H
