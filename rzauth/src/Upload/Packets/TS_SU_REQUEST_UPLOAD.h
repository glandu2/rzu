#ifndef TS_SU_REQUEST_UPLOAD_H
#define TS_SU_REQUEST_UPLOAD_H

#include "Packets/PacketBaseMessage.h"

#pragma pack(push, 1)
struct TS_SU_REQUEST_UPLOAD : public TS_MESSAGE
{
	int32_t client_id;
	int32_t account_id;
	int32_t guild_sid;
	int32_t one_time_password;
	uint8_t type;

	static const uint16_t packetID = 50003;
};
#pragma pack(pop)

#endif // TS_SU_REQUEST_UPLOAD_H
