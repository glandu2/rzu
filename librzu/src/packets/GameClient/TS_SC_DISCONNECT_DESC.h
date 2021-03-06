#pragma once

#include "Packet/PacketDeclaration.h"

enum TS_SC_DISCONNECT_TYPE : uint8_t
{
	DISCONNECT_TYPE_ANOTHER_LOGIN = 0,
	DISCONNECT_TYPE_DUPLICATED_LOGIN = 1,
	DISCONNECT_TYPE_BILLING_EXPIRED = 2,
	DISCONNECT_TYPE_GAME_ADDICTION = 3,
	DISCONNECT_TYPE_DB_ERROR = 100,
	DISCONNECT_TYPE_ANTI_HACK = 101,
	DISCONNECT_TYPE_SCRIPT = 102
};


#define TS_SC_DISCONNECT_DESC_DEF(_) \
	_(simple)(TS_SC_DISCONNECT_TYPE, desc_id)

#define TS_SC_DISCONNECT_DESC_ID(X) \
	X(28, version < EPIC_9_6_3) \
	X(1028, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_SC_DISCONNECT_DESC, SessionType::GameClient, SessionPacketOrigin::Server);
#undef TS_SC_DISCONNECT_DESC_DEF

