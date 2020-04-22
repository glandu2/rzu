#pragma once

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

