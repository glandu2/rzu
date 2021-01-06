#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SC_DEATHMATCH_RANKING_REOPEN_DEF(_) \
	_(simple)(int32_t, time_epoch)

#define TS_SC_DEATHMATCH_RANKING_REOPEN_ID(X) \
	X(4302, true)

CREATE_PACKET_VER_ID(TS_SC_DEATHMATCH_RANKING_REOPEN, SessionType::GameClient, SessionPacketOrigin::Server);
#undef TS_SC_DEATHMATCH_RANKING_REOPEN_DEF

