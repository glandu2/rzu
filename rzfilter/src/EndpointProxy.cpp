#include "EndpointProxy.h"

EndpointProxy::EndpointProxy(IFilterEndpoint* endpoint) : endpoint(endpoint) {
	cachedPacketVersion = endpoint->getPacketVersion();
	cachedStreamAddress = endpoint->getAddress();
	cachedStrictForward = endpoint->isStrictForwardEnabled();
}

int EndpointProxy::getPacketVersion() {
	if(endpoint)
		return endpoint->getPacketVersion();

	return cachedPacketVersion;
}

void EndpointProxy::sendPacket(const TS_MESSAGE* packet) {
	if(endpoint)
		endpoint->sendPacket(packet);
	else
		Object::logStatic(
		    Object::LL_Error, "EndpointProxy", "Can't send packet id %d, no underlying endpoint\n", packet->id);
}

void EndpointProxy::sendPacket(MessageBuffer& packet) {
	sendPacket(reinterpret_cast<const TS_MESSAGE*>(packet.getData()));
}

void EndpointProxy::close() {
	if(endpoint)
		endpoint->close();
}

StreamAddress EndpointProxy::getAddress() {
	if(endpoint)
		return endpoint->getAddress();

	return cachedStreamAddress;
}

void EndpointProxy::banAddress(StreamAddress address) {
	if(endpoint)
		endpoint->banAddress(address);
}

bool EndpointProxy::isStrictForwardEnabled() {
	if(endpoint)
		return endpoint->isStrictForwardEnabled();

	return cachedStrictForward;
}

void EndpointProxy::detach() {
	endpoint = nullptr;
}
