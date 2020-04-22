#pragma once

#include "Packet/PacketBaseMessage.h"

class IFilterEndpoint {
public:
	virtual void sendPacket(const TS_MESSAGE* packet) = 0;
	virtual int getPacketVersion() = 0;
	virtual void close() = 0;
	virtual StreamAddress getAddress() = 0;
	virtual void banAddress(StreamAddress address) = 0;
	virtual bool isStrictForwardEnabled() = 0;

	template<class T> typename std::enable_if<!std::is_pointer<T>::value, int>::type sendPacket(const T& data) {
		int version = getPacketVersion();
		MessageBuffer buffer(data.getSize(version), version);
		data.serialize(&buffer);
		int id = buffer.getMessageId();
		sendPacket(buffer);
		return id;
	}

protected:
	virtual void sendPacket(MessageBuffer& packet) = 0;
};

