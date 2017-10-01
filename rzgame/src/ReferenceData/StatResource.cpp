#include "StatResource.h"
#include "Config/GlobalConfig.h"
#include <iterator>

template<> void DbQueryJob<GameServer::StatResourceBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              CONFIG_GET()->game.arcadia.connectionString,
	              "select * from StatResource",
	              DbQueryBinding::EM_MultiRows);

	addColumn("id", &OutputType::id);
	addColumn("str", &OutputType::strength);
	addColumn("vit", &OutputType::vitality);
	addColumn("dex", &OutputType::dexterity);
	addColumn("agi", &OutputType::agility);
	addColumn("int", &OutputType::intelligence);
	addColumn("men", &OutputType::wisdom);
	addColumn("luk", &OutputType::luck);
}
DECLARE_DB_BINDING(GameServer::StatResourceBinding, "statresource");
