#include "FilterEndpoint.h"
#include "FilterProxy.h"

int FilterEndpoint::getPacketVersion() {
	return filter->getPacketVersion();
}

void FilterEndpoint::sendPacket(const TS_MESSAGE* packet) {
	if(toClient) {
		filter->onServerPacket(packet);
	} else {
		filter->onClientPacket(packet);
	}
}

void FilterEndpoint::sendPacket(MessageBuffer& packet) {
	sendPacket(reinterpret_cast<const TS_MESSAGE*>(packet.getData()));
}
