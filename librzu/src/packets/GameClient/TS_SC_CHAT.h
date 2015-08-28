#ifndef PACKETS_TS_SC_CHAT_H
#define PACKETS_TS_SC_CHAT_H

#include "Packet/PacketBaseMessage.h"
#include "PacketEnums.h"

#pragma pack(push, 1)
struct TS_SC_CHAT : public TS_MESSAGE
{
	char szSender[21];
	unsigned short len;
	unsigned char type;
	char message[0];

	static const int packetID = 22;
};
#pragma pack(pop)

#endif // PACKETS_TS_SC_CHAT_H
