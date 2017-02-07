#ifndef PACKETS_TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER_H
#define PACKETS_TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER_DEF(_) \
	_(simple)(bool, bSuccess)

CREATE_PACKET(TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER, 4719);

#endif // PACKETS_TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER_H
