#pragma once

#include "Database/DbQueryJob.h"
#include "uv.h"
#include <stdint.h>

class DbConnectionPool;

namespace AuthServer {

struct DB_UpdateLastServerIdx {
	struct Input {
		Input(uint32_t accountId, uint16_t lastLoginServerIdx)
		    : accountId(accountId), lastLoginServerIdx(lastLoginServerIdx) {}
		Input() {}

		uint32_t accountId;
		uint16_t lastLoginServerIdx;
	};

	struct Output {};
};

}  // namespace AuthServer

