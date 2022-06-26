#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_UC_LOGIN_RESULT_DEF(_) \
	_(simple)(uint16_t, result)

CREATE_PACKET(TS_UC_LOGIN_RESULT, 50006, SessionType::UploadClient, SessionPacketOrigin::Server);
