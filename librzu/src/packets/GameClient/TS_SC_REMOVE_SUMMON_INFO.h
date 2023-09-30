#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_SC_REMOVE_SUMMON_INFO_DEF(_) \
	_(simple)(ar_handle_t, card_handle)

#define TS_SC_REMOVE_SUMMON_INFO_ID(X) \
	X(302, version < EPIC_9_6_3) \
	X(1302, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_SC_REMOVE_SUMMON_INFO, SessionType::GameClient, SessionPacketOrigin::Server);
#undef TS_SC_REMOVE_SUMMON_INFO_DEF

