#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CU_DOWNLOAD_ICON_DEF(_) \
	_(simple)(uint32_t, guild_id) \
	_(simple)(uint32_t, icon_size) \
	_(string)(guild_name, 64) \
	_(string)(file_name, 64)

CREATE_PACKET(TS_CU_DOWNLOAD_ICON, 50009, SessionType::UploadClient, SessionPacketOrigin::Client);
