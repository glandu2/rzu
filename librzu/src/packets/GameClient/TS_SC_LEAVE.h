#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_SC_LEAVE_DEF(_) \
	_(simple)(ar_handle_t, handle)

#define TS_SC_LEAVE_ID(X) \
	X(9, version < EPIC_9_6_3) \
	X(1009, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_SC_LEAVE, SessionType::GameClient, SessionPacketOrigin::Server);
#undef TS_SC_LEAVE_DEF

