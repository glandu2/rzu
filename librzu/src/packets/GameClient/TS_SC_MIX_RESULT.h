#ifndef PACKETS_TS_SC_MIX_RESULT_H
#define PACKETS_TS_SC_MIX_RESULT_H

#include "Packet/PacketDeclaration.h"

enum TS_MIX_TYPE : int8_t
{
	MIX_TYPE_NONE = 0,
	MIX_TYPE_AWAKEN = 1
};

#define TS_SC_MIX_RESULT_DEF(_) \
	_(count)(uint32_t, handles) \
	_(simple)(TS_MIX_TYPE, type, version >= EPIC_8_1, MIX_TYPE_NONE) \
	_(dynarray)(ar_handle_t, handles)

CREATE_PACKET(TS_SC_MIX_RESULT, 257);
#undef TS_SC_MIX_RESULT_DEF

#endif // PACKETS_TS_SC_MIX_RESULT_H
