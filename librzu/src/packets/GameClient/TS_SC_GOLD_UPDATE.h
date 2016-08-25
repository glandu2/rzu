#ifndef PACKETS_TS_SC_GOLD_UPDATE_H
#define PACKETS_TS_SC_GOLD_UPDATE_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_GOLD_UPDATE_DEF(_) \
	_(simple)(def) (uint64_t, gold) \
	_(simple)(impl)(uint64_t, gold, version >= EPIC_5_1) \
	_(simple)(impl)(uint32_t, gold, version < EPIC_5_1) \
	_(simple) (uint32_t, chaos, version > EPIC_5_1)

CREATE_PACKET(TS_SC_GOLD_UPDATE, 1001);

#endif // PACKETS_TS_SC_GOLD_UPDATE_H
