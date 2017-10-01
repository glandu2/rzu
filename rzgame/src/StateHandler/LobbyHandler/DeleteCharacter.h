#ifndef AUTHSERVER_DELETE_CHARACTER_H
#define AUTHSERVER_DELETE_CHARACTER_H

#include "Database/DbQueryJob.h"
#include "GameTypes.h"

namespace GameServer {

struct DeleteCharacterBinding {
	struct Input {
		std::string character_name;
		game_sid_t out_character_sid;
	};

	struct Output {};
};

}  // namespace GameServer

#endif  // AUTHSERVER_DELETE_CHARACTER_H
