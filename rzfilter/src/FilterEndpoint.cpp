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

void FilterEndpoint::close() {
	if(toClient) {
		filter->getClientEndpoint()->close();
	} else {
		filter->getServerEndpoint()->close();
	}
}

StreamAddress FilterEndpoint::getAddress() {
	if(toClient) {
		return filter->getClientEndpoint()->getAddress();
	} else {
		return filter->getServerEndpoint()->getAddress();
	}
}

void FilterEndpoint::banAddress(StreamAddress address) {
	filter->getClientEndpoint()->banAddress(address);
}

bool FilterEndpoint::isStrictForwardEnabled() {
	if(toClient) {
		return filter->getClientEndpoint()->isStrictForwardEnabled();
	} else {
		return filter->getServerEndpoint()->isStrictForwardEnabled();
	}
}
