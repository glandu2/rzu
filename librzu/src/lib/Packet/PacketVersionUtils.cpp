#include "PacketVersionUtils.h"
#include "Config/GlobalCoreConfig.h"

namespace PacketVersionUtils {

std::string getDefaultAuthVersionString(packet_version_t version) {
	if(version >= EPIC_9_6_6)
		return "20210128";
	else if(version >= EPIC_9_2)
		return "201507080";
	else if(version >= EPIC_4_1)
		return "200701120";
	else
		return "200609280";
}

std::string getDefaultGameVersionString(packet_version_t version) {
	if(version >= EPIC_9_7_2)
		return "20220622";
	else if(version >= EPIC_9_7_1)
		return "20220210";
	else if(version >= EPIC_9_6_6)
		return "20210128";
	else if(version >= EPIC_9_6_4)
		return "20200922";
	else if(version >= EPIC_9_6_3)
		return "20200713";
	else if(version >= EPIC_9_6)
		return "20190102";
	else if(version >= EPIC_9_5_3)
		return "20181211";
	else if(version >= EPIC_9_5_2)
		return "20180117";
	else if(version >= EPIC_9_4_AR)
		return "205001120";
	else if(version >= EPIC_9_2)
		return "201507080";
	else if(version >= EPIC_4_1)
		return "200701120";
	else
		return "200609280";
}

std::string getAuthVersionString(packet_version_t version) {
	if(!GlobalCoreConfig::get()->client.authVersion.isDefault())
		return GlobalCoreConfig::get()->client.authVersion;

	return getDefaultAuthVersionString(version);
}

std::string getGameVersionString(packet_version_t version) {
	if(!GlobalCoreConfig::get()->client.gameVersion.isDefault())
		return GlobalCoreConfig::get()->client.gameVersion;

	return getDefaultGameVersionString(version);
}

}  // namespace PacketVersionUtils
