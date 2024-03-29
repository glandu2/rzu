#pragma once

#include "Packet/PacketDeclaration.h"
#include "PacketEnums.h"

// Last tested: EPIC_9_8_1

enum TS_LOGIN_SUCCESS_FLAG
{
	LSF_EULA_ACCEPTED = 0x1,
	LSF_ACCOUNT_BLOCK_WARNING = 0x2,
	LSF_DISTRIBUTION_CODE_REQUIRED = 0x4
};

#define TS_AC_RESULT_DEF(_) \
	_(simple)(uint16_t, request_msg_id) \
	_(simple)(uint16_t, result) \
	_(simple)(int32_t, login_flag)
CREATE_PACKET(TS_AC_RESULT, 10000, SessionType::AuthClient, SessionPacketOrigin::Server);

