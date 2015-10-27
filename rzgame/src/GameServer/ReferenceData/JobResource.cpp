#include "JobResource.h"
#include "../../GlobalConfig.h"
#include <iterator>

template<> void DbQueryJob<GameServer::JobResourceBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.arcadia.connectionString,
				  "select * from JobResource",
				  DbQueryBinding::EM_MultiRows);

	addColumn("id", &OutputType::id);
	addColumn("stati_id", &OutputType::stat_id);
	addColumn("job_class", &OutputType::job_class);
	addColumn("job_depth", &OutputType::job_depth);
	addColumn("up_lv", &OutputType::up_lv);
	addColumn("up_jlv", &OutputType::up_jlv);
	addColumn("available_job_0", &OutputType::available_job, 0);
	addColumn("available_job_1", &OutputType::available_job, 1);
	addColumn("available_job_2", &OutputType::available_job, 2);
	addColumn("available_job_3", &OutputType::available_job, 3);
}
DECLARE_DB_BINDING(GameServer::JobResourceBinding, "jobresource");
