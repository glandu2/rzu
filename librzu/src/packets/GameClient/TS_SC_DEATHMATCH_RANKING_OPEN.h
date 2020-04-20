#ifndef PACKETS_TS_SC_DEATHMATCH_RANKING_OPEN_H
#define PACKETS_TS_SC_DEATHMATCH_RANKING_OPEN_H

#include "Packet/PacketDeclaration.h"
#include "TS_SC_DEATHMATCH_RANKING.h"

#define TS_SC_DEATHMATCH_RANKING_OPEN_DEF(_) \
	_(array)(TS_DEATHMATCH_RANKING_INFO, players, 5)

CREATE_PACKET(TS_SC_DEATHMATCH_RANKING_OPEN, 4300);
#undef TS_SC_DEATHMATCH_RANKING_OPEN_DEF

#endif // PACKETS_TS_SC_DEATHMATCH_RANKING_OPEN_H
