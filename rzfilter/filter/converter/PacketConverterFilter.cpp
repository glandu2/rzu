#include "PacketConverterFilter.h"
#include "PacketIterator.h"

PacketConverterFilter::PacketConverterFilter(IFilterEndpoint* client,
                                             IFilterEndpoint* server,
                                             ServerType serverType,
                                             PacketConverterFilter*)
    : IFilter(client, server, serverType) {}

PacketConverterFilter::~PacketConverterFilter() {}

bool PacketConverterFilter::onServerPacket(const TS_MESSAGE* packet) {
	if(serverType == ST_Game)
		return convertPacketAndSend(SessionType::GameClient, server, client, packet, server->getPacketVersion(), true);
	else
		return convertPacketAndSend(SessionType::AuthClient, server, client, packet, server->getPacketVersion(), true);
}

bool PacketConverterFilter::onClientPacket(const TS_MESSAGE* packet) {
	if(serverType == ST_Game)
		return convertPacketAndSend(SessionType::GameClient, client, server, packet, client->getPacketVersion(), false);
	else
		return convertPacketAndSend(SessionType::AuthClient, client, server, packet, client->getPacketVersion(), false);
}

template<typename Packet>
bool sendPacket(IFilterEndpoint* source, IFilterEndpoint* target, const TS_MESSAGE* packet, int version) {
	Packet pkt = {};
	if(packet->process(pkt, version)) {
		if(packet->id != Packet::getId(version))
			Object::logStatic(Object::LL_Warning,
			                  "rzfilter_version_converter",
			                  "Packet %s id mismatch, got %d, expected %d for version 0x%06x\n",
			                  Packet::getName(),
			                  packet->id,
			                  Packet::getId(version),
			                  version);
		target->sendPacket(pkt);
		return false;  // packet sent, no need to forward the original
	} else {
		Object::logStatic(Object::LL_Warning,
		                  "rzfilter_version_converter",
		                  "Can't parse packet %s id %d with version 0x%X\n",
		                  Packet::getName(),
		                  packet->id,
		                  version);
		return !source->isStrictForwardEnabled();  // packet not sent, need to forward the original
	}
}

template<class Packet> struct SendPacketFunctor {
	bool operator()(IFilterEndpoint* source, IFilterEndpoint* target, const TS_MESSAGE* packet, int version) {
		return sendPacket<Packet>(source, target, packet, version);
	}
};

bool PacketConverterFilter::convertPacketAndSend(SessionType sessionType,
                                                 IFilterEndpoint* source,
                                                 IFilterEndpoint* target,
                                                 const TS_MESSAGE* packet,
                                                 int version,
                                                 bool isServerMsg) {
	bool packetProcessed;

	if(sessionType == SessionType::AuthClient)
		packetProcessed = specificPacketConverter.convertAuthPacketAndSend(client, server, packet, isServerMsg);
	else
		packetProcessed =
		    specificPacketConverter.convertGamePacketAndSend(target, packet, source->getPacketVersion(), isServerMsg);

	if(packetProcessed) {
		if(packet->id == 9999) {
			// This is a dummy packet, ignore it
			return false;
		}

		bool packetNeedToBeForwarded = false;
		bool packetProcessed =
		    processPacket<SendPacketFunctor>(sessionType,
		                                     isServerMsg ? SessionPacketOrigin::Server : SessionPacketOrigin::Client,
		                                     version,
		                                     packet->id,
		                                     packetNeedToBeForwarded,
		                                     source,
		                                     target,
		                                     packet,
		                                     version);

		if(!packetProcessed) {
			Object::logStatic(Object::LL_Warning, "rzfilter_version_converter", "packet id %d unknown\n", packet->id);

			// Unknown packet, forward the same raw packet
			return !source->isStrictForwardEnabled();
		} else {
			return packetNeedToBeForwarded;
		}
	}

	return false;
}

IFilter* createFilter(IFilterEndpoint* client,
                      IFilterEndpoint* server,
                      IFilter::ServerType serverType,
                      IFilter* oldFilter) {
	Object::logStatic(Object::LL_Info, "rzfilter_version_converter", "Loaded filter from data: %p\n", oldFilter);
	return new PacketConverterFilter(client, server, serverType, (PacketConverterFilter*) oldFilter);
}

void destroyFilter(IFilter* filter) {
	delete filter;
}
