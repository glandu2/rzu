#ifndef PACKETS_TS_SC_SKILLCARD_INFO_H
#define PACKETS_TS_SC_SKILLCARD_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_SKILLCARD_INFO_DEF(_) \
	_(simple)(ar_handle_t, item_handle) \
	_(simple)(ar_handle_t, target_handle)

CREATE_PACKET(TS_SC_SKILLCARD_INFO, 286);

#endif // PACKETS_TS_SC_SKILLCARD_INFO_H
