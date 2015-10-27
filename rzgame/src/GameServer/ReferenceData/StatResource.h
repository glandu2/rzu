#ifndef STATRESOURCE_H
#define STATRESOURCE_H

#include "RefDataLoader.h"
#include "Database/DbQueryJobCallback.h"
#include <unordered_map>
#include <stdint.h>

namespace GameServer {

struct StatResource {
	int32_t id;
	int32_t strength;
	int32_t vitality;
	int32_t dexterity;
	int32_t agility;
	int32_t intelligence;
	int32_t wisdom;
	int32_t luck;
	//int32_t text_id;
	//int32_t icon_id;
	//std::string icon_file_name;
};

class StatResourceBinding : public RefDataLoaderHelper<StatResource, StatResourceBinding> {
	DECLARE_CLASS(GameServer::StatResourceBinding)
};

}

#endif // STATRESOURCE_H
