#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CU_LOGIN_DEF(_) \
	_(simple)(uint32_t, client_id) \
	_(simple)(uint32_t, account_id) \
	_(simple)(uint32_t, guild_id) \
	_(simple)(uint32_t, one_time_password) \
	_(string)(raw_server_name, 32)

CREATE_PACKET(TS_CU_LOGIN, 50005, SessionType::UploadClient, SessionPacketOrigin::Client);
