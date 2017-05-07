#ifndef AUTHSERVER_CHARACTER_WEAR_INFO_H
#define AUTHSERVER_CHARACTER_WEAR_INFO_H

#include "Database/DbQueryJob.h"
#include "GameTypes.h"

namespace GameServer {

class CharacterWearInfo {
public:
	 game_sid_t character_sid;
	 int32_t wear_info;
	 uint32_t code;
	 uint32_t enhance;
	 uint32_t level;
	 uint8_t elemental_effect_type;
	 uint32_t appearance_code;

	 CharacterWearInfo() { wear_info = -1; }
};

struct CharacterWearInfoBinding {
	struct Input {
		uint32_t account_id;
	};

	typedef CharacterWearInfo Output;
};

}

#endif // AUTHSERVER_CHARACTER_WEAR_INFO_H
