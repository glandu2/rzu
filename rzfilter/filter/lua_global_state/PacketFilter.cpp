#include "PacketFilter.h"
#include "Core/Utils.h"
#include "LuaTableWriter.h"
#include "LuaTimer.h"
#include "LuaUtils.h"
#include "PacketIterator.h"

static const char* LUA_MAIN_FILE = "rzfilter.lua";

static void pushEnum(lua_State* L, const char* name, int value) {
	lua_pushinteger(L, value);
	lua_setglobal(L, name);
}

template<class T> struct LuaDeclarePacketTypeNameCallback {
	void operator()(lua_State* L) { pushEnum(L, T::getName(), T::getId(EPIC_LATEST)); }
};

template<class T> struct LuaSerializePackerCallback {
	bool operator()(lua_State* L, int version, const TS_MESSAGE* packet) {
		T deserializedPacket;
		MessageBuffer buffer((void*) packet, packet->size, version);

		deserializedPacket.deserialize(&buffer);
		if(buffer.checkPacketFinalSize()) {
			LuaTableWriter luaSerializer(L, version);
			luaSerializer.prepareWrite();
			deserializedPacket.serialize(&luaSerializer);
			lua_pushstring(L, T::getName());
			lua_setfield(L, -2, "__name");

			// Set id with latest number to be able to compare with constants
			lua_pushinteger(L, packet->id);
			lua_setfield(L, -2, "__id");
			lua_pushinteger(L, T::getId(EPIC_LATEST));
			lua_setfield(L, -2, "id");

			return true;
		} else {
			return false;
		}
	}
};

GlobalLuaState::GlobalLuaState()
    : luaFunctions{&lua_onServerPacketFunction,
                   &lua_onClientPacketFunction,
                   &lua_onUnknownPacketFunction,
                   &lua_onClientConnectedFunction,
                   &lua_onServerDisconnectedFunction,
                   &lua_onClientDisconnectedFunction},
      currentLuaFileMtime(0),
      newLuaFileMtime(0) {
	struct stat info;
	int ret = stat(LUA_MAIN_FILE, &info);
	if(ret == 0 && (info.st_mode & S_IFREG))
		newLuaFileMtime = currentLuaFileMtime = info.st_mtime;

	createLuaVM();
	initLuaVM();

	reloadCheckTimer.start(this, &GlobalLuaState::onCheckReload, 5000, 5000);
	reloadCheckTimer.unref();
}

GlobalLuaState::~GlobalLuaState() {
	deinitLuaVM();
	closeLuaVM();
}

GlobalLuaState* GlobalLuaState::getInstance() {
	static GlobalLuaState globalLuaState;
	return &globalLuaState;
}

lua_State* GlobalLuaState::getLuaState() {
	return L;
}

bool GlobalLuaState::luaCallPacket(int function,
                                   const char* functionName,
                                   int clientEndpoint,
                                   int serverEndpoint,
                                   const TS_MESSAGE* packet,
                                   int version,
                                   bool isServerPacket,
                                   SessionType sessionType,
                                   bool isStrictForwardEnabled) {
	if(packet->id == 9999)
		return false;

	if(function != LUA_REFNIL && function != LUA_NOREF) {
		bool returnValue = true;

		int topBeforeCall = lua_gettop(L);

		lua_pushcfunction(L, &luaMessageHandler);

		lua_rawgeti(L, LUA_REGISTRYINDEX, function);
		lua_rawgeti(L, LUA_REGISTRYINDEX, clientEndpoint);
		lua_rawgeti(L, LUA_REGISTRYINDEX, serverEndpoint);
		if(!pushPacket(L, packet, version, isServerPacket, sessionType)) {
			Object::logStatic(
			    Object::LL_Error, "rzfilter_lua_global_state_module", "Cannot deserialize packet id: %d\n", packet->id);
			lua_settop(L, topBeforeCall);
			return luaCallUnknownPacket(
			    clientEndpoint, serverEndpoint, packet, isServerPacket, sessionType, isStrictForwardEnabled);
		}
		lua_pushinteger(L, (int) sessionType);

		int result = lua_pcall(L, 4, 1, -6);
		if(result) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_global_state_module",
			                  "Cannot run lua %s function: %d:%s\n",
			                  functionName,
			                  result,
			                  lua_tostring(L, -1));
			lua_pop(L, 1);
		} else {
			if(lua_isboolean(L, -1)) {
				returnValue = lua_toboolean(L, -1) != 0;
			}
		}

		lua_settop(L, topBeforeCall);

		return returnValue;
	}

	return true;
}

bool GlobalLuaState::luaCallUnknownPacket(int clientEndpoint,
                                          int serverEndpoint,
                                          const TS_MESSAGE* packet,
                                          bool isServerPacket,
                                          SessionType sessionType,
                                          bool isStrictForwardEnabled) {
	bool returnValue = !isStrictForwardEnabled;

	if(lua_onUnknownPacketFunction.ref != LUA_REFNIL && lua_onUnknownPacketFunction.ref != LUA_NOREF) {
		int topBeforeCall = lua_gettop(L);

		lua_pushcfunction(L, &luaMessageHandler);

		lua_rawgeti(L, LUA_REGISTRYINDEX, lua_onUnknownPacketFunction.ref);
		lua_rawgeti(L, LUA_REGISTRYINDEX, clientEndpoint);
		lua_rawgeti(L, LUA_REGISTRYINDEX, serverEndpoint);
		lua_pushinteger(L, packet->id);
		lua_pushinteger(L, (int) sessionType);
		lua_pushboolean(L, isServerPacket);

		int result = lua_pcall(L, 5, 1, -7);
		if(result) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_global_state_module",
			                  "Cannot run lua onUnknownPacket function: %d:%s\n",
			                  result,
			                  lua_tostring(L, -1));
			lua_pop(L, 1);
		} else {
			if(lua_isboolean(L, -1)) {
				returnValue = lua_toboolean(L, -1) != 0;
			}
		}

		lua_settop(L, topBeforeCall);
	}

	return returnValue;
}

bool GlobalLuaState::luaCallOnConnectionEvent(int luaOnEventFunction,
                                              const char* functionName,
                                              int clientEndpoint,
                                              int serverEndpoint) {
	if(luaOnEventFunction != LUA_REFNIL && luaOnEventFunction != LUA_NOREF) {
		int topBeforeCall = lua_gettop(L);

		lua_pushcfunction(L, &luaMessageHandler);

		lua_rawgeti(L, LUA_REGISTRYINDEX, luaOnEventFunction);
		lua_rawgeti(L, LUA_REGISTRYINDEX, clientEndpoint);
		lua_rawgeti(L, LUA_REGISTRYINDEX, serverEndpoint);

		int result = lua_pcall(L, 2, 0, -4);
		if(result) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_global_state_module",
			                  "Cannot run lua %s function: %d:%s\n",
			                  functionName,
			                  result,
			                  lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		lua_settop(L, topBeforeCall);
		return true;
	}

	return false;
}

int GlobalLuaState::luaMessageHandler(lua_State* L) {
	const char* msg = lua_tostring(L, 1);
	luaL_traceback(L, L, msg, 1);
	return 1;
}

bool GlobalLuaState::onClientConnected(int clientEndpoint, int serverEndpoint) {
	return luaCallOnConnectionEvent(
	    lua_onClientConnectedFunction.ref, "onClientConnected", clientEndpoint, serverEndpoint);
}

bool GlobalLuaState::onServerPacket(int clientEndpoint,
                                    int serverEndpoint,
                                    const TS_MESSAGE* packet,
                                    int packetVersion,
                                    SessionType sessionType,
                                    bool isStrictForwardEnabled) {
	return luaCallPacket(lua_onServerPacketFunction.ref,
	                     "onServerPacket",
	                     clientEndpoint,
	                     serverEndpoint,
	                     packet,
	                     packetVersion,
	                     true,
	                     sessionType,
	                     isStrictForwardEnabled);
}

bool GlobalLuaState::onClientPacket(int clientEndpoint,
                                    int serverEndpoint,
                                    const TS_MESSAGE* packet,
                                    int packetVersion,
                                    SessionType sessionType,
                                    bool isStrictForwardEnabled) {
	return luaCallPacket(lua_onClientPacketFunction.ref,
	                     "onClientPacket",
	                     clientEndpoint,
	                     serverEndpoint,
	                     packet,
	                     packetVersion,
	                     false,
	                     sessionType,
	                     isStrictForwardEnabled);
}

bool GlobalLuaState::onServerDisconnected(int clientEndpoint, int serverEndpoint) {
	return luaCallOnConnectionEvent(
	    lua_onServerDisconnectedFunction.ref, "onServerDisconnected", clientEndpoint, serverEndpoint);
}

bool GlobalLuaState::onClientDisconnected(int clientEndpoint, int serverEndpoint) {
	return luaCallOnConnectionEvent(
	    lua_onClientDisconnectedFunction.ref, "onClientDisconnected", clientEndpoint, serverEndpoint);
}

void GlobalLuaState::closeLuaVM() {
	if(L) {
		lua_close(L);
		L = nullptr;
	}
}

void GlobalLuaState::createLuaVM() {
	closeLuaVM();

	L = luaL_newstate();

	luaL_openlibs(L);

	// Override lua print function to output via the log system
	lua_pushcfunction(L, &GlobalLuaState::luaPrintToLogs);
	lua_setglobal(L, "print");

	iteratePackets<LuaDeclarePacketTypeNameCallback>(L);
	pushEnum(L, "ST_Auth", (int) SessionType::AuthClient);
	pushEnum(L, "ST_Game", (int) SessionType::GameClient);
	pushEnum(L, "ST_Upload", (int) SessionType::UploadClient);

	LuaEndpoint::initLua(L);
	luaL_requiref(L, "timer", &LuaTimer::initLua, true);
	luaL_requiref(L, "utils", &LuaUtils::initLua, true);
}

void GlobalLuaState::initLuaVM() {
	deinitLuaVM();

	for(auto luaFunction : luaFunctions) {
		luaFunction->ref = LUA_NOREF;

		lua_pushnil(L);
		lua_setglobal(L, luaFunction->name);
	}

	lua_pushcfunction(L, &luaMessageHandler);

	const char* const filename = LUA_MAIN_FILE;
	int result = luaL_loadfilex(L, filename, nullptr);
	if(result) {
		Object::logStatic(Object::LL_Error,
		                  "rzfilter_lua_global_state_module",
		                  "Cannot load lua file %s: %s\n",
		                  filename,
		                  lua_tostring(L, -1));
		lua_pop(L, 2);  // remove the error object and the message handler
		return;
	}

	result = lua_pcall(L, 0, 0, -2);
	if(result) {
		Object::logStatic(Object::LL_Error,
		                  "rzfilter_lua_global_state_module",
		                  "Cannot run lua file %s: %s\n",
		                  filename,
		                  lua_tostring(L, -1));
		lua_pop(L, 2);  // remove the error object and the message handler
		return;
	}
	lua_pop(L, 1);  // remove the message handler

	for(auto luaFunction : luaFunctions) {
		lua_getglobal(L, luaFunction->name);
		if(lua_isfunction(L, -1)) {
			luaFunction->ref = luaL_ref(L, LUA_REGISTRYINDEX);
			lua_pushnil(L);  // for the next pop as luaL_ref pop the value
		} else {
			luaFunction->ref = LUA_NOREF;
			Object::logStatic(Object::LL_Info,
			                  "rzfilter_lua_global_state_module",
			                  "Lua register: %s must be a lua function matching its C counterpart: %s\n",
			                  luaFunction->name,
			                  luaFunction->signature);
		}
		lua_pop(L, 1);
	}
}

void GlobalLuaState::deinitLuaVM() {
	if(L) {
		for(auto luaFunction : luaFunctions) {
			if(luaFunction->ref != LUA_NOREF) {
				luaL_unref(L, LUA_REGISTRYINDEX, luaFunction->ref);
				luaFunction->ref = LUA_NOREF;
			}
		}
	}
}

int GlobalLuaState::luaPrintToLogs(lua_State* L) {
	int nargs = lua_gettop(L);

	for(int i = 1; i <= nargs; i++) {
		const char* str = lua_tostring(L, i);
		Object::logStatic(Object::LL_Info, "rzfilter_lua_global_state_module", "Lua message: %s\n", str);
	}

	return 0;
}

bool GlobalLuaState::pushPacket(
    lua_State* L, const TS_MESSAGE* packet, int version, bool isServer, SessionType sessionType) {
	bool ok;
	bool serializationSucess = false;
	SessionPacketOrigin origin;

	if(isServer)
		origin = SessionPacketOrigin::Server;
	else
		origin = SessionPacketOrigin::Client;

	ok = processPacket<LuaSerializePackerCallback>(
	    sessionType, origin, version, packet->id, serializationSucess, L, version, packet);

	return ok && serializationSucess;
}

void GlobalLuaState::onCheckReload() {
	struct stat info;
	int ret = stat(LUA_MAIN_FILE, &info);
	if(ret == 0 && (info.st_mode & S_IFREG)) {
		// If different time as current and has not changed since previous check, do reload
		if(newLuaFileMtime != currentLuaFileMtime && newLuaFileMtime == info.st_mtime) {
			Object::logStatic(Object::LL_Info, "rzfilter_lua_global_state_module", "Reloading %s\n", LUA_MAIN_FILE);
			initLuaVM();
			currentLuaFileMtime = info.st_mtime;
		} else if(currentLuaFileMtime != info.st_mtime) {
			Object::logStatic(Object::LL_Info,
			                  "rzfilter_lua_global_state_module",
			                  "%s changed, will reload next time\n",
			                  LUA_MAIN_FILE);
		}
		newLuaFileMtime = info.st_mtime;
	}
}

PacketFilter::PacketFilter(IFilterEndpoint* client, IFilterEndpoint* server, SessionType sessionType, PacketFilter*)
    : IFilter(client, server, sessionType) {
	clientEndpoint =
	    LuaEndpoint::createInstance(GlobalLuaState::getInstance()->getLuaState(), client, false, sessionType);
	lua_clientEndpointRef = luaL_ref(GlobalLuaState::getInstance()->getLuaState(), LUA_REGISTRYINDEX);
	serverEndpoint =
	    LuaEndpoint::createInstance(GlobalLuaState::getInstance()->getLuaState(), server, true, sessionType);
	lua_serverEndpointRef = luaL_ref(GlobalLuaState::getInstance()->getLuaState(), LUA_REGISTRYINDEX);

	GlobalLuaState::getInstance()->onClientConnected(lua_clientEndpointRef, lua_serverEndpointRef);
}

PacketFilter::~PacketFilter() {
	if(clientEndpoint) {
		clientEndpoint->detachEndpoint();
		luaL_unref(GlobalLuaState::getInstance()->getLuaState(), LUA_REGISTRYINDEX, lua_clientEndpointRef);
	}
	if(serverEndpoint) {
		serverEndpoint->detachEndpoint();
		luaL_unref(GlobalLuaState::getInstance()->getLuaState(), LUA_REGISTRYINDEX, lua_serverEndpointRef);
	}
}

bool PacketFilter::onServerPacket(const TS_MESSAGE* packet) {
	return GlobalLuaState::getInstance()->onServerPacket(lua_clientEndpointRef,
	                                                     lua_serverEndpointRef,
	                                                     packet,
	                                                     server->getPacketVersion(),
	                                                     sessionType,
	                                                     server->isStrictForwardEnabled());
}

bool PacketFilter::onClientPacket(const TS_MESSAGE* packet) {
	return GlobalLuaState::getInstance()->onClientPacket(lua_clientEndpointRef,
	                                                     lua_serverEndpointRef,
	                                                     packet,
	                                                     client->getPacketVersion(),
	                                                     sessionType,
	                                                     client->isStrictForwardEnabled());
}

void PacketFilter::onServerDisconnected() {
	GlobalLuaState::getInstance()->onServerDisconnected(lua_clientEndpointRef, lua_serverEndpointRef);

	IFilter::onServerDisconnected();
}

void PacketFilter::onClientDisconnected() {
	if(!GlobalLuaState::getInstance()->onClientDisconnected(lua_clientEndpointRef, lua_serverEndpointRef)) {
		IFilter::onClientDisconnected();
	}

	clientEndpoint->detachEndpoint();
	luaL_unref(GlobalLuaState::getInstance()->getLuaState(), LUA_REGISTRYINDEX, lua_clientEndpointRef);
	clientEndpoint = nullptr;
}

IFilter* createFilter(IFilterEndpoint* client, IFilterEndpoint* server, SessionType sessionType, IFilter* oldFilter) {
	Object::logStatic(Object::LL_Info, "rzfilter_lua_global_state_module", "Loaded filter from data: %p\n", oldFilter);
	return new PacketFilter(client, server, sessionType, (PacketFilter*) oldFilter);
}

void destroyFilter(IFilter* filter) {
	delete filter;
}

void* initializeGlobalFilter() {
	GlobalLuaState::getInstance();
	return nullptr;
}

void destroyGlobalFilter(void* globalData) {}
