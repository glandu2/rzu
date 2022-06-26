#include "PacketFilter.h"
#include "Core/Utils.h"
#include "Packet/JSONWriter.h"
#include "PacketIterator.h"
#include <sstream>

PacketFilter::PacketFilter(IFilterEndpoint* client,
                           IFilterEndpoint* server,
                           SessionType sessionType,
                           PacketFilter* oldFilter)
    : IFilter(client, server, sessionType) {
	if(oldFilter) {
		data = oldFilter->data;
		oldFilter->data = nullptr;
	} else {
		data = new Data;
	}
}

PacketFilter::~PacketFilter() {
	if(data)
		delete data;
}

bool PacketFilter::onServerPacket(const TS_MESSAGE* packet) {
	printPacketJson(sessionType, packet, server->getPacketVersion(), true);

	return true;
}

bool PacketFilter::onClientPacket(const TS_MESSAGE* packet) {
	printPacketJson(sessionType, packet, server->getPacketVersion(), false);

	return true;
}

void PacketFilter::printPacketJson(SessionType sessionType, const TS_MESSAGE* packet, int version, bool isServerMsg) {
	if(packet->id == 9999) {
		// This is a dummy packet, ignore it
		return;
	}

	bool dummy;
	bool packetProcessed =
	    processPacket<PrintPacketFunctor>(sessionType,
	                                      isServerMsg ? SessionPacketOrigin::Server : SessionPacketOrigin::Client,
	                                      version,
	                                      packet->id,
	                                      dummy,
	                                      this,
	                                      packet,
	                                      version);

	if(!packetProcessed) {
		Object::logStatic(Object::LL_Warning, "rzfilter_json", "packet id %d unknown\n", packet->id);
	}
}

IFilter* createFilter(IFilterEndpoint* client, IFilterEndpoint* server, SessionType sessionType, IFilter* oldFilter) {
	Object::logStatic(Object::LL_Info, "rzfilter_json", "Loaded filter from data: %p\n", oldFilter);
	return new PacketFilter(client, server, sessionType, (PacketFilter*) oldFilter);
}

void destroyFilter(IFilter* filter) {
	delete filter;
}

template<class Packet> void PacketFilter::showPacketJson(const Packet* packet, int version) {
	JSONWriter jsonWriter(version, false);
	packet->serialize(&jsonWriter);
	jsonWriter.finalize();
	std::string jsonData = jsonWriter.toString();

	Object::logStatic(Object::LL_Info, "rzfilter_json", "%s packet:\n%s\n", Packet::getName(), jsonData.c_str());
}

template<class Packet>
bool PacketFilter::PrintPacketFunctor<Packet>::operator()(PacketFilter* filter, const TS_MESSAGE* packet, int version) {
	packet->process(filter, &PacketFilter::showPacketJson<Packet>, version, version);
	return true;
}
