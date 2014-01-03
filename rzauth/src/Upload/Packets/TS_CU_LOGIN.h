#ifndef TS_CU_LOGIN_H
#define TS_CU_LOGIN_H

#include "Packets/PacketBaseMessage.h"

#pragma pack(push, 1)
struct TS_CU_LOGIN : public TS_MESSAGE
{
	int32_t client_id;
	int32_t account_id;
	int32_t guild_id;
	int32_t one_time_password;
	char raw_server_name[32];

	static const uint16_t packetID = 50005;
};
#pragma pack(pop)

#endif // TS_CU_LOGIN_H
