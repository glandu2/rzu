#pragma once

#include "Packet/PacketBaseMessage.h"

#pragma pack(push, 1)
struct TS_CU_DOWNLOAD_ICON : public TS_MESSAGE
{
	uint32_t guild_id;
	uint32_t icon_size;
	char guild_name[64];
	char file_name[64];

	static const uint16_t packetID = 50009;
};
#pragma pack(pop)

