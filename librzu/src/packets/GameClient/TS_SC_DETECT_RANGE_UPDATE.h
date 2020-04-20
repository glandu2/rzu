#ifndef PACKETS_TS_SC_DETECT_RANGE_UPDATE_H
#define PACKETS_TS_SC_DETECT_RANGE_UPDATE_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_DETECT_RANGE_UPDATE_DEF(_) \
	_(simple) (ar_handle_t, handle) \
	_(simple) (float, detect_range)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_DETECT_RANGE_UPDATE, 1005);
#undef TS_SC_DETECT_RANGE_UPDATE_DEF

#endif // PACKETS_TS_SC_DETECT_RANGE_UPDATE_H
