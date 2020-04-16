#ifndef PACKETS_TS_SC_DEATHMATCH_RANKING_H
#define PACKETS_TS_SC_DEATHMATCH_RANKING_H

#include "Packet/PacketDeclaration.h"

#define TS_DEATHMATCH_RANKING_INFO_DEF(_) \
	_(simple)(int32_t, rank) \
	_(simple)(uint16_t, kills) \
	_(simple)(uint16_t, deaths) \
	_(simple)(uint32_t, score) \
	_(simple)(uint32_t, unknown_2) \
	_(string)(name, 16)

CREATE_STRUCT(TS_DEATHMATCH_RANKING_INFO);

#define TS_SC_DEATHMATCH_RANKING_DEF(_) \
	_(simple)(int32_t, time_epoch) \
	_(array)(TS_DEATHMATCH_RANKING_INFO, players, 30)

CREATE_PACKET(TS_SC_DEATHMATCH_RANKING, 4301);

#endif // PACKETS_TS_SC_DEATHMATCH_RANKING_H