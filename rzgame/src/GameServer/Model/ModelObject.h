#ifndef MODELOBJECT_H
#define MODELOBJECT_H

#include "../GameTypes.h"
#include <unordered_map>
#include "Core/Object.h"

namespace GameServer {

class ClientSession;

template<class ModelType, game_handle_t HANDLE_MASK>
class ModelObject : public Object {
public:
	~ModelObject() {
		freeHandles.push_back(handle);
		dataObjectsByHandle.erase(handle);
		dataObjectsBySid.erase(sid);
		handle = 0;
		sid = 0;
	}

	static bool addObject(game_sid_t sid, ModelType* model) {
		ModelObject* object = model;

		std::pair<std::unordered_map<game_sid_t, ModelType*>::iterator, bool> idResult = dataObjectsBySid.insert(std::pair<game_sid_t, ModelType*>(sid, model));
		if(!idResult.second)
			return false;

		auto result2 = dataObjectsByHandle.insert(std::pair<game_handle_t, ModelType*>(object->handle, model));
		if(!result2.second) {
			object->log(LL_Error, "Duplicate handle %d !!!\n", object->handle);
			dataObjectsBySid.erase(sid);
			return false;
		}

		object->sid = sid;

		return true;
	}

	static ModelType* getObjectBySid(game_sid_t id) {
		auto it = dataObjectsBySid.find(id);
		if(it != dataObjectsBySid.end())
			return it->second;

		return nullptr;
	}

	static ModelType* getObjectByHandle(game_handle_t handle) {
		auto it = dataObjectsByHandle.find(handle);
		if(it != dataObjectsByHandle.end())
			return it->second;

		return nullptr;
	}
	game_sid_t sid;
	game_handle_t handle;

protected:
	ModelObject() : sid(0), handle(allocHandle()) {}


private:

	static game_handle_t allocHandle() {
		if(freeHandles.empty()) {
			nextHandle = (nextHandle+1) & 0x0FFFFFFF;
			return nextHandle | (HANDLE_MASK << 28);
		} else {
			game_handle_t ret = freeHandles.back();
			freeHandles.pop_back();
			return ret;
		}
	}

	static std::unordered_map<game_sid_t, ModelType*> dataObjectsBySid;
	static std::unordered_map<game_handle_t, ModelType*> dataObjectsByHandle;
	static game_handle_t nextHandle;
	static std::vector<game_handle_t> freeHandles;
};

template<class ModelType, game_handle_t HANDLE_MASK>
std::unordered_map<game_sid_t, ModelType*> ModelObject<ModelType, HANDLE_MASK>::dataObjectsBySid;

template<class ModelType, game_handle_t HANDLE_MASK>
std::unordered_map<game_handle_t, ModelType*> ModelObject<ModelType, HANDLE_MASK>::dataObjectsByHandle;

template<class ModelType, game_handle_t HANDLE_MASK>
game_handle_t ModelObject<ModelType, HANDLE_MASK>::nextHandle = 0;

template<class ModelType, game_handle_t HANDLE_MASK>
std::vector<game_handle_t> ModelObject<ModelType, HANDLE_MASK>::freeHandles;

}

#endif // MODELOBJECT_H
