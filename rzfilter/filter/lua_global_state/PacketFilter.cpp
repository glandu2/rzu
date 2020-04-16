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

GlobalLuaState::GlobalLuaState() : currentLuaFileMtime(0), newLuaFileMtime(0) {
	struct stat info;
	int ret = stat(LUA_MAIN_FILE, &info);
	if(ret == 0 && (info.st_mode & S_IFREG))
		newLuaFileMtime = currentLuaFileMtime = info.st_mtime;

	initLuaVM();

	reloadCheckTimer.start(this, &GlobalLuaState::onCheckReload, 5000, 5000);
	reloadCheckTimer.unref();
}

GlobalLuaState::~GlobalLuaState() {
	deinitLuaVM();
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
                                   IFilter::ServerType serverType,
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
		if(!pushPacket(L, packet, version, isServerPacket, serverType)) {
			Object::logStatic(
			    Object::LL_Error, "rzfilter_lua_global_state_module", "Cannot deserialize packet id: %d\n", packet->id);
			lua_settop(L, topBeforeCall);
			return luaCallUnknownPacket(
			    clientEndpoint, serverEndpoint, packet, isServerPacket, serverType, isStrictForwardEnabled);
		}
		lua_pushinteger(L, serverType);

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
                                          IFilter::ServerType serverType,
                                          bool isStrictForwardEnabled) {
	bool returnValue = !isStrictForwardEnabled;

	if(lua_onUnknownPacketFunction != LUA_REFNIL && lua_onUnknownPacketFunction != LUA_NOREF) {
		int topBeforeCall = lua_gettop(L);

		lua_pushcfunction(L, &luaMessageHandler);

		lua_rawgeti(L, LUA_REGISTRYINDEX, lua_onUnknownPacketFunction);
		lua_rawgeti(L, LUA_REGISTRYINDEX, clientEndpoint);
		lua_rawgeti(L, LUA_REGISTRYINDEX, serverEndpoint);
		lua_pushinteger(L, packet->id);
		lua_pushinteger(L, serverType);
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

bool GlobalLuaState::luaCallOnDisconnected(int luaOnDisconnectedFunction,
                                           const char* functionName,
                                           int clientEndpoint,
                                           int serverEndpoint) {
	if(luaOnDisconnectedFunction != LUA_REFNIL && luaOnDisconnectedFunction != LUA_NOREF) {
		int topBeforeCall = lua_gettop(L);

		lua_pushcfunction(L, &luaMessageHandler);

		lua_rawgeti(L, LUA_REGISTRYINDEX, luaOnDisconnectedFunction);
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

bool GlobalLuaState::onServerPacket(int clientEndpoint,
                                    int serverEndpoint,
                                    const TS_MESSAGE* packet,
                                    int packetVersion,
                                    IFilter::ServerType serverType,
                                    bool isStrictForwardEnabled) {
	return luaCallPacket(lua_onServerPacketFunction,
	                     "onServerPacket",
	                     clientEndpoint,
	                     serverEndpoint,
	                     packet,
	                     packetVersion,
	                     true,
	                     serverType,
	                     isStrictForwardEnabled);
}

bool GlobalLuaState::onClientPacket(int clientEndpoint,
                                    int serverEndpoint,
                                    const TS_MESSAGE* packet,
                                    int packetVersion,
                                    IFilter::ServerType serverType,
                                    bool isStrictForwardEnabled) {
	return luaCallPacket(lua_onClientPacketFunction,
	                     "onClientPacket",
	                     clientEndpoint,
	                     serverEndpoint,
	                     packet,
	                     packetVersion,
	                     false,
	                     serverType,
	                     isStrictForwardEnabled);
}

bool GlobalLuaState::onClientDisconnected(int clientEndpoint, int serverEndpoint) {
	return luaCallOnDisconnected(
	    lua_onClientDisconnectedFunction, "onClientDisconnected", clientEndpoint, serverEndpoint);
}

void GlobalLuaState::initLuaVM() {
	deinitLuaVM();

	L = luaL_newstate();
	lua_onServerPacketFunction = LUA_NOREF;
	lua_onClientPacketFunction = LUA_NOREF;
	lua_onUnknownPacketFunction = LUA_NOREF;
	lua_onClientDisconnectedFunction = LUA_NOREF;

	luaL_openlibs(L);

	// Override lua print function to output via the log system
	lua_pushcfunction(L, &GlobalLuaState::luaPrintToLogs);
	lua_setglobal(L, "print");

	iteratePackets<LuaDeclarePacketTypeNameCallback>(L);
	pushEnum(L, "ST_Auth", IFilter::ST_Auth);
	pushEnum(L, "ST_Game", IFilter::ST_Game);

	LuaEndpoint::initLua(L);
	luaL_requiref(L, "timer", &LuaTimer::initLua, true);
	luaL_requiref(L, "utils", &LuaUtils::initLua, true);

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

	lua_getglobal(L, "onServerPacket");
	if(lua_isfunction(L, -1)) {
		lua_onServerPacketFunction = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);  // for the next pop as luaL_ref pop the value
	} else {
		lua_onServerPacketFunction = LUA_NOREF;
		Object::logStatic(Object::LL_Info,
		                  "rzfilter_lua_global_state_module",
		                  "Lua register: onServerPacket must be a lua function matching its C counterpart: "
		                  "bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* "
		                  "packet, ServerType serverType)\n");
	}
	lua_pop(L, 1);

	lua_getglobal(L, "onClientPacket");
	if(lua_isfunction(L, -1)) {
		lua_onClientPacketFunction = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);  // for the next pop as luaL_ref pop the value
	} else {
		lua_onClientPacketFunction = LUA_NOREF;
		Object::logStatic(Object::LL_Info,
		                  "rzfilter_lua_global_state_module",
		                  "Lua register: onClientPacket must be a lua function matching its C counterpart: "
		                  "bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* "
		                  "packet, ServerType serverType)\n");
	}
	lua_pop(L, 1);

	lua_getglobal(L, "onUnknownPacket");
	if(lua_isfunction(L, -1)) {
		lua_onUnknownPacketFunction = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);  // for the next pop as luaL_ref pop the value
	} else {
		lua_onUnknownPacketFunction = LUA_NOREF;
		Object::logStatic(Object::LL_Info,
		                  "rzfilter_lua_global_state_module",
		                  "Lua register: onUnknownPacket must be a lua function matching its C counterpart: "
		                  "bool onUnknownPacket(IFilterEndpoint* fromEndpoint, IFilterEndpoint* toEndpoint, int "
		                  "packetId, ServerType serverType, bool isServerPacket)\n");
	}
	lua_pop(L, 1);

	lua_getglobal(L, "onClientDisconnected");
	if(lua_isfunction(L, -1)) {
		lua_onClientDisconnectedFunction = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);  // for the next pop as luaL_ref pop the value
	} else {
		lua_onClientDisconnectedFunction = LUA_NOREF;
		Object::logStatic(Object::LL_Info,
		                  "rzfilter_lua_global_state_module",
		                  "Lua register: onClientDisconnected must be a lua function matching its C counterpart: "
		                  "bool onClientDisconnected(IFilterEndpoint* client, IFilterEndpoint* server)\n");
	}
	lua_pop(L, 1);
}

void GlobalLuaState::deinitLuaVM() {
	if(L) {
		if(lua_onServerPacketFunction != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, lua_onServerPacketFunction);
			lua_onServerPacketFunction = LUA_NOREF;
		}
		if(lua_onClientPacketFunction != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, lua_onClientPacketFunction);
			lua_onClientPacketFunction = LUA_NOREF;
		}
		if(lua_onUnknownPacketFunction != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, lua_onUnknownPacketFunction);
			lua_onUnknownPacketFunction = LUA_NOREF;
		}
		if(lua_onClientDisconnectedFunction != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, lua_onClientDisconnectedFunction);
			lua_onClientDisconnectedFunction = LUA_NOREF;
		}
		lua_close(L);
		L = nullptr;
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
    lua_State* L, const TS_MESSAGE* packet, int version, bool isServer, IFilter::ServerType serverType) {
	bool ok;
	bool serializationSucess = false;
	SessionType sessionType;
	SessionPacketOrigin origin;

	if(serverType == IFilter::ST_Auth)
		sessionType = SessionType::AuthClient;
	else
		sessionType = SessionType::GameClient;

	if(isServer)
		origin = SessionPacketOrigin::Server;
	else
		origin = SessionPacketOrigin::Client;

	ok = processPacket<LuaSerializePackerCallback>(
	    sessionType, origin, packet->id, serializationSucess, L, version, packet);

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

PacketFilter::PacketFilter(IFilterEndpoint* client, IFilterEndpoint* server, ServerType serverType, PacketFilter*)
    : IFilter(client, server, serverType) {
	clientEndpoint =
	    LuaEndpoint::createInstance(GlobalLuaState::getInstance()->getLuaState(), client, false, serverType);
	lua_clientEndpointRef = luaL_ref(GlobalLuaState::getInstance()->getLuaState(), LUA_REGISTRYINDEX);
	serverEndpoint =
	    LuaEndpoint::createInstance(GlobalLuaState::getInstance()->getLuaState(), server, true, serverType);
	lua_serverEndpointRef = luaL_ref(GlobalLuaState::getInstance()->getLuaState(), LUA_REGISTRYINDEX);
}

PacketFilter::~PacketFilter() {
	if(clientEndpoint) {
		clientEndpoint->detachEndpoint();
		luaL_unref(GlobalLuaState::getInstance()->getLuaState(), LUA_REGISTRYINDEX, lua_clientEndpointRef);
	}
	serverEndpoint->detachEndpoint();
	luaL_unref(GlobalLuaState::getInstance()->getLuaState(), LUA_REGISTRYINDEX, lua_serverEndpointRef);
}

bool PacketFilter::onServerPacket(const TS_MESSAGE* packet) {
	return GlobalLuaState::getInstance()->onServerPacket(lua_clientEndpointRef,
	                                                     lua_serverEndpointRef,
	                                                     packet,
	                                                     server->getPacketVersion(),
	                                                     serverType,
	                                                     server->isStrictForwardEnabled());
}

bool PacketFilter::onClientPacket(const TS_MESSAGE* packet) {
	return GlobalLuaState::getInstance()->onClientPacket(lua_clientEndpointRef,
	                                                     lua_serverEndpointRef,
	                                                     packet,
	                                                     client->getPacketVersion(),
	                                                     serverType,
	                                                     client->isStrictForwardEnabled());
}

void PacketFilter::onClientDisconnected() {
	if(!GlobalLuaState::getInstance()->onClientDisconnected(lua_clientEndpointRef, lua_serverEndpointRef)) {
		IFilter::onClientDisconnected();
	}

	clientEndpoint->detachEndpoint();
	luaL_unref(GlobalLuaState::getInstance()->getLuaState(), LUA_REGISTRYINDEX, lua_clientEndpointRef);
	clientEndpoint = nullptr;
}

IFilter* createFilter(IFilterEndpoint* client,
                      IFilterEndpoint* server,
                      IFilter::ServerType serverType,
                      IFilter* oldFilter) {
	Object::logStatic(Object::LL_Info, "rzfilter_lua_global_state_module", "Loaded filter from data: %p\n", oldFilter);
	return new PacketFilter(client, server, serverType, (PacketFilter*) oldFilter);
}

void destroyFilter(IFilter* filter) {
	delete filter;
}

void* initializeGlobalFilter() {
	return nullptr;
}

void destroyGlobalFilter(void* globalData) {}
