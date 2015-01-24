#ifndef TS_CS_UPDATE_H
#define TS_CS_UPDATE_H

#include "Packets/PacketBaseMessage.h"

#pragma pack(push, 1)
struct TS_CS_UPDATE : public TS_MESSAGE
{
	unsigned int handle;
	static const int packetID = 503;
};
#pragma pack(pop)

#endif // TS_CS_UPDATE_H