#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_TIMESYNC_DEF(_) \
	_(simple) (ar_time_t, time)

#define TS_TIMESYNC_ID(X) \
	X(2, version < EPIC_9_6_3) \
	X(1002, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_TIMESYNC, SessionType::GameClient, SessionPacketOrigin::Any);
#undef TS_TIMESYNC_DEF

