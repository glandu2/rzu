#ifndef PACKETS_TS_SC_BATTLE_ARENA_JOIN_QUEUE_H
#define PACKETS_TS_SC_BATTLE_ARENA_JOIN_QUEUE_H

#include "Packet/PacketDeclaration.h"
#include "TS_SC_BATTLE_ARENA_BATTLE_INFO.h"

#define TS_SC_BATTLE_ARENA_JOIN_QUEUE_DEF(_) \
	_(simple)(int32_t, nArenaID) \
	_(simple)(TS_BATTLE_GRADE, eGrade)

CREATE_PACKET(TS_SC_BATTLE_ARENA_JOIN_QUEUE, 4702);

#endif // PACKETS_TS_SC_BATTLE_ARENA_JOIN_QUEUE_H
