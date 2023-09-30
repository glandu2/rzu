#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_UC_DOWNLOAD_ICON_DEF(_) \
	_(simple)(uint32_t, guild_id) \
	_(simple)(uint32_t, icon_size) \
	_(string)(guild_name, 64) \
	_(string)(file_name, 64) \
	_(endarray)(uint8_t, file_contents)

CREATE_PACKET(TS_UC_DOWNLOAD_ICON, 50010, SessionType::UploadClient, SessionPacketOrigin::Server);

