#ifndef DB_UPDATELASTSERVERIDX_H
#define DB_UPDATELASTSERVERIDX_H

#include "Object.h"
#include "uv.h"
#include <string>
#include <stdint.h>
#include "Log.h"
#include "DbQueryJob.h"

class DbConnectionPool;

namespace AuthServer {

class DB_UpdateLastServerIdx : public DbQueryJob<DB_UpdateLastServerIdx>
{
	DECLARE_CLASS(AuthServer::DB_UpdateLastServerIdx)

public:
	static bool init(DbConnectionPool* dbConnectionPool);
	static void deinit();

	DB_UpdateLastServerIdx(uint32_t accountId, uint16_t lastLoginServerIdx);

private:
	uint32_t accountId;
	uint16_t lastLoginServerIdx;
};

} // namespace AuthServer

#endif // DB_UPDATELASTSERVERIDX_H
