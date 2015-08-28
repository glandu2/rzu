#ifndef IFILTERENDPOINT_H
#define IFILTERENDPOINT_H

#include "Packet/PacketBaseMessage.h"

class IFilterEndpoint
{
public:
	virtual void sendPacket(const TS_MESSAGE* packet) = 0;
};

#endif // IFILTERENDPOINT_H
