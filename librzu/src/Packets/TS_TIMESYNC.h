#ifndef PACKETS_TS_TIMESYNC_H
#define PACKETS_TS_TIMESYNC_H

#include "Packets/PacketBaseMessage.h"
#include "Packets/PacketEnums.h"

#pragma pack(push, 1)
struct TS_TIMESYNC : public TS_MESSAGE
{
	unsigned int time;
	static const int packetID = 2;
};
#pragma pack(pop)

#endif // PACKETS_TS_TIMESYNC_H