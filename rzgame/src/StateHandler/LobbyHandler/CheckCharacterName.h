#ifndef AUTHSERVER_CHECK_CHARACTER_NAME_H
#define AUTHSERVER_CHECK_CHARACTER_NAME_H

#include "Database/DbQueryJob.h"
#include <stdint.h>

namespace GameServer {

struct CheckCharacterNameBinding {
	struct Input {
		std::string character_name;
	};

	struct Output {};
};

}  // namespace GameServer

#endif  // AUTHSERVER_CHECK_CHARACTER_NAME_H
