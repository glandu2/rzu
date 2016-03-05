#ifndef IFILTERENDPOINT_H
#define IFILTERENDPOINT_H

#include "Packet/PacketBaseMessage.h"

class IFilterEndpoint
{
public:
	virtual void sendPacket(const TS_MESSAGE* packet) = 0;
	virtual int getPacketVersion() = 0;

	template<class T> typename std::enable_if<!std::is_pointer<T>::value, void>::type
	sendPacket(const T& data) {
		int version = getPacketVersion();
		MessageBuffer buffer(data.getSize(version), version);
		data.serialize(&buffer);
		sendPacket(buffer);
	}

protected:
	virtual void sendPacket(MessageBuffer& packet) = 0;
};

#endif // IFILTERENDPOINT_H
