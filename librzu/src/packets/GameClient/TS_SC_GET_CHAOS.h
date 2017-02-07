#ifndef PACKETS_TS_SC_GET_CHAOS_H
#define PACKETS_TS_SC_GET_CHAOS_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_GET_CHAOS_DEF(_) \
	_(simple)(uint32_t, hPlayer) \
	_(simple)(uint32_t, hCorpse) \
	_(simple)(int32_t, nChaos) \
	_(simple)(int8_t, nBonusType) \
	_(simple)(int8_t, nBonusPercent) \
	_(simple)(int32_t, nBonus)

CREATE_PACKET(TS_SC_GET_CHAOS, 213);

#endif // PACKETS_TS_SC_GET_CHAOS_H
