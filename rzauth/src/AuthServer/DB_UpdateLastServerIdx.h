#ifndef AUTHSERVER_DB_UPDATELASTSERVERIDX_H
#define AUTHSERVER_DB_UPDATELASTSERVERIDX_H

#include "uv.h"
#include <stdint.h>
#include "Database/DbQueryJob.h"

class DbConnectionPool;

namespace AuthServer {

struct DB_UpdateLastServerIdx
{
	struct Input {
		Input(uint32_t accountId, uint16_t lastLoginServerIdx) : accountId(accountId), lastLoginServerIdx(lastLoginServerIdx) {}
		Input() {}

		uint32_t accountId;
		uint16_t lastLoginServerIdx;
	};

	struct Output {};
};

} // namespace AuthServer

#endif // AUTHSERVER_DB_UPDATELASTSERVERIDX_H
