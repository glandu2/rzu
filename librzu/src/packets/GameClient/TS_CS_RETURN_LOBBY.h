#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CS_RETURN_LOBBY_DEF(_)

#define TS_CS_RETURN_LOBBY_ID(X) \
	X(23, version < EPIC_9_6_3) \
	X(1023, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_CS_RETURN_LOBBY, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_RETURN_LOBBY_DEF

