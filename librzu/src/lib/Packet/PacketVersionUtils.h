#pragma once

#include "../Extern.h"
#include "PacketEpics.h"
#include <string>

namespace PacketVersionUtils {

std::string RZU_EXTERN getAuthVersionString(packet_version_t version);
std::string RZU_EXTERN getGameVersionString(packet_version_t version);

}  // namespace PacketVersionUtils
