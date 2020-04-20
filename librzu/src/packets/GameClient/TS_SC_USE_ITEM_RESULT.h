#ifndef PACKETS_TS_SC_USE_ITEM_RESULT_H
#define PACKETS_TS_SC_USE_ITEM_RESULT_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_USE_ITEM_RESULT_DEF(_) \
	_(simple)(ar_handle_t, item_handle) \
	_(simple)(ar_handle_t, target_handle)

CREATE_PACKET(TS_SC_USE_ITEM_RESULT, 283);
#undef TS_SC_USE_ITEM_RESULT_DEF

#endif // PACKETS_TS_SC_USE_ITEM_RESULT_H
