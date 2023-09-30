#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CS_INSTANCE_GAME_EXIT_DEF(_)

// Since EPIC_6_3
#define TS_CS_INSTANCE_GAME_EXIT_ID(X) \
	X(4251, true)

CREATE_PACKET_VER_ID(TS_CS_INSTANCE_GAME_EXIT, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_INSTANCE_GAME_EXIT_DEF

