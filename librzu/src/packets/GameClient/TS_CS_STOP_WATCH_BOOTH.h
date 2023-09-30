#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CS_STOP_WATCH_BOOTH_DEF(_) \
	_(simple)(ar_handle_t, target)

#define TS_CS_STOP_WATCH_BOOTH_ID(X) \
	X(704, version < EPIC_9_6_3) \
	X(1704, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_CS_STOP_WATCH_BOOTH, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_STOP_WATCH_BOOTH_DEF

