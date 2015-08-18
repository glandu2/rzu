#ifndef IFILTERENDPOINT_H
#define IFILTERENDPOINT_H

#include "PacketBaseMessage.h"

class IFilterEndpoint
{
public:
	virtual void sendPacket(const TS_MESSAGE* packet) = 0;
};

#endif // IFILTERENDPOINT_H
