#include "PacketFilter.h"
#include "Core/Utils.h"
#include "LuaTableWriter.h"
#include "LuaTimer.h"
#include "LuaUtils.h"
#include "PacketIterator.h"

static const char* LUA_MAIN_FILE = "rzfilter.lua";

template<class T> struct SendPacketFromLuaCallback {
	bool operator()(lua_State* L, IFilterEndpoint* endpoint) {
		return LuaEndpointMetaTable::sendPacketFromLua<T>(L, endpoint);
	}
};
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

const char* const LuaEndpointMetaTable::NAME = "rzfilter.IFilterEndpoint";
const luaL_Reg LuaEndpointMetaTable::FUNCTIONS[] = {{"getPacketVersion", &getPacketVersion},
                                                    {"sendPacket", &sendPacket},
                                                    {"close", &close},
                                                    {"getIp", &getIp},
                                                    {"banIp", &banIp},
                                                    {NULL, NULL}};

int LuaEndpointMetaTable::getPacketVersion(lua_State* L) {
	LuaEndpointMetaTable* self = (LuaEndpointMetaTable*) luaL_checkudata(L, 1, NAME);
	if(!self || !self->endpoint)
		return 0;

	lua_pushinteger(L, self->endpoint->getPacketVersion());
	return 1;
}

int LuaEndpointMetaTable::sendPacket(lua_State* L) {
	LuaEndpointMetaTable* self = (LuaEndpointMetaTable*) luaL_checkudata(L, 1, NAME);

	if(!self || !self->endpoint)
		return 0;

	IFilterEndpoint* endpoint = self->endpoint;
	luaL_checktype(L, 2, LUA_TTABLE);

	lua_getfield(L, 2, "id");
	if(!lua_isinteger(L, -1)) {
		luaL_error(L, "Bad packet field type for id, expected integer, got %s", lua_typename(L, lua_type(L, -1)));
	}

	int packetId = (int) lua_tointeger(L, -1);
	lua_pop(L, 1);

	bool ok = true;
	SessionType sessionType = self->sessionType;
	SessionPacketOrigin origin;

	if(self->isServerEndpoint)
		origin = SessionPacketOrigin::Client;
	else
		origin = SessionPacketOrigin::Server;

	if(!processPacket<SendPacketFromLuaCallback>(sessionType, origin, EPIC_LATEST, packetId, ok, L, endpoint)) {
		luaL_error(L,
		           "Packet id unknown: %d for session type %s and origin %s\n",
		           packetId,
		           sessionType == SessionType::AuthClient ? "AuthClient" : "GameClient",
		           origin == SessionPacketOrigin::Client ? "Client" : "Server");
		lua_pushboolean(L, false);
	} else if(!ok) {
		luaL_error(L, "Couln't send packet %d", packetId);
		lua_pushboolean(L, false);
	} else {
		lua_pushboolean(L, true);
	}

	return 1;
}

int LuaEndpointMetaTable::close(lua_State* L) {
	LuaEndpointMetaTable* self = (LuaEndpointMetaTable*) luaL_checkudata(L, 1, NAME);
	if(!self || !self->endpoint)
		return 0;

	self->endpoint->close();
	return 0;
}

int LuaEndpointMetaTable::getIp(lua_State* L) {
	LuaEndpointMetaTable* self = (LuaEndpointMetaTable*) luaL_checkudata(L, 1, NAME);
	if(!self || !self->endpoint)
		return 0;

	StreamAddress address = self->endpoint->getAddress();
	char ip[108];
	address.getName(ip, sizeof(ip));
	lua_pushstring(L, ip);
	return 1;
}

int LuaEndpointMetaTable::banIp(lua_State* L) {
	LuaEndpointMetaTable* self = (LuaEndpointMetaTable*) luaL_checkudata(L, 1, NAME);

	if(!self || !self->endpoint)
		return 0;

	luaL_checktype(L, 2, LUA_TSTRING);

	const char* ipStr = lua_tostring(L, 2);
	StreamAddress address;
	address.port = 0;
	address.setFromName(ipStr);

	self->endpoint->banAddress(address);

	return 0;
}

template<class T> bool LuaEndpointMetaTable::sendPacketFromLua(lua_State* L, IFilterEndpoint* endpoint) {
	T packet;
	LuaTableWriter luaDeserialiser(L, endpoint->getPacketVersion());

	packet.deserialize(&luaDeserialiser);

	if(!luaDeserialiser.getProcessStatus())
		return false;

	endpoint->sendPacket(packet);

	return true;
}

PacketFilter::PacketFilter(IFilterEndpoint* client, IFilterEndpoint* server, SessionType sessionType, PacketFilter*)
    : IFilter(client, server, sessionType),
      clientEndpoint(client, false, sessionType),
      serverEndpoint(server, true, sessionType),
      currentLuaFileMtime(0),
      newLuaFileMtime(0) {
	struct stat info;
	int ret = stat(LUA_MAIN_FILE, &info);
	if(ret == 0 && (info.st_mode & S_IFREG))
		newLuaFileMtime = currentLuaFileMtime = info.st_mtime;

	initLuaVM();

	reloadCheckTimer.start(this, &PacketFilter::onCheckReload, 5000, 5000);
}

PacketFilter::~PacketFilter() {
	deinitLuaVM();
}

bool PacketFilter::onServerPacket(const TS_MESSAGE* packet) {
	return luaCallPacket(lua_onServerPacketFunction, "onServerPacket", packet, server->getPacketVersion(), true);
}

bool PacketFilter::onClientPacket(const TS_MESSAGE* packet) {
	return luaCallPacket(lua_onClientPacketFunction, "onClientPacket", packet, client->getPacketVersion(), false);
}

void PacketFilter::onServerDisconnected() {
	luaCallOnDisconnected(lua_onServerDisconnectedFunction, "onServerDisconnected");

	IFilter::onServerDisconnected();
}

void PacketFilter::onClientDisconnected() {
	if(!luaCallOnDisconnected(lua_onClientDisconnectedFunction, "onClientDisconnected")) {
		IFilter::onClientDisconnected();
	}
}

bool PacketFilter::luaCallPacket(
    int function, const char* functionName, const TS_MESSAGE* packet, int version, bool isServerPacket) {
	if(packet->id == 9999)
		return false;

	if(function != LUA_REFNIL && function != LUA_NOREF) {
		bool returnValue = true;

		int topBeforeCall = lua_gettop(L);

		lua_pushcfunction(L, &luaMessageHandler);

		lua_rawgeti(L, LUA_REGISTRYINDEX, function);
		pushEndpoint(L, &clientEndpoint);
		pushEndpoint(L, &serverEndpoint);
		if(!pushPacket(L, packet, version, isServerPacket)) {
			Object::logStatic(
			    Object::LL_Error, "rzfilter_lua_module", "Cannot deserialize packet id: %d\n", packet->id);
			lua_settop(L, topBeforeCall);
			return luaCallUnknownPacket(packet, isServerPacket);
		}
		lua_pushinteger(L, (int) sessionType);

		int result = lua_pcall(L, 4, 1, -6);
		if(result) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_module",
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

bool PacketFilter::luaCallUnknownPacket(const TS_MESSAGE* packet, bool isServerPacket) {
	bool returnValue;

	if(isServerPacket)
		returnValue = !server->isStrictForwardEnabled();
	else
		returnValue = !client->isStrictForwardEnabled();

	if(lua_onUnknownPacketFunction != LUA_REFNIL && lua_onUnknownPacketFunction != LUA_NOREF) {
		int topBeforeCall = lua_gettop(L);

		lua_pushcfunction(L, &luaMessageHandler);

		lua_rawgeti(L, LUA_REGISTRYINDEX, lua_onUnknownPacketFunction);
		pushEndpoint(L, &clientEndpoint);
		pushEndpoint(L, &serverEndpoint);
		lua_pushinteger(L, packet->id);
		lua_pushinteger(L, (int) sessionType);
		lua_pushboolean(L, isServerPacket);

		int result = lua_pcall(L, 5, 1, -7);
		if(result) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_module",
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

bool PacketFilter::luaCallOnDisconnected(int luaOnDisconnectedFunction, const char* functionName) {
	if(luaOnDisconnectedFunction != LUA_REFNIL && luaOnDisconnectedFunction != LUA_NOREF) {
		int topBeforeCall = lua_gettop(L);

		lua_pushcfunction(L, &luaMessageHandler);

		lua_rawgeti(L, LUA_REGISTRYINDEX, luaOnDisconnectedFunction);
		pushEndpoint(L, &clientEndpoint);
		pushEndpoint(L, &serverEndpoint);

		int result = lua_pcall(L, 2, 0, -4);
		if(result) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_module",
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

int PacketFilter::luaMessageHandler(lua_State* L) {
	const char* msg = lua_tostring(L, 1);
	luaL_traceback(L, L, msg, 1);
	return 1;
}

void PacketFilter::initLuaVM() {
	deinitLuaVM();

	L = luaL_newstate();
	lua_onServerPacketFunction = LUA_NOREF;
	lua_onClientPacketFunction = LUA_NOREF;
	lua_onUnknownPacketFunction = LUA_NOREF;
	lua_onServerDisconnectedFunction = LUA_NOREF;
	lua_onClientDisconnectedFunction = LUA_NOREF;

	luaL_openlibs(L);

	// Override lua print function to output via the log system
	lua_pushcfunction(L, &PacketFilter::luaPrintToLogs);
	lua_setglobal(L, "print");

	iteratePackets<LuaDeclarePacketTypeNameCallback>(L);
	pushEnum(L, "ST_Auth", (int) SessionType::AuthClient);
	pushEnum(L, "ST_Game", (int) SessionType::GameClient);
	pushEnum(L, "ST_Upload", (int) SessionType::UploadClient);

	// LuaEndpointMetaTable metatable wrapping IFilterEndpoint
	luaL_newmetatable(L, LuaEndpointMetaTable::NAME);

	// set __index as a table with custom methods (like sendGamePacket)
	lua_pushstring(L, "__index");
	lua_newtable(L);
	luaL_setfuncs(L, LuaEndpointMetaTable::FUNCTIONS, 0);
	lua_settable(L, -3);

	// pop metatable
	lua_pop(L, 1);

	luaL_requiref(L, "timer", &LuaTimer::initLua, true);
	luaL_requiref(L, "utils", &LuaUtils::initLua, true);

	lua_pushcfunction(L, &luaMessageHandler);

	const char* const filename = LUA_MAIN_FILE;
	int result = luaL_loadfilex(L, filename, nullptr);
	if(result) {
		Object::logStatic(
		    Object::LL_Error, "rzfilter_lua_module", "Cannot load lua file %s: %s\n", filename, lua_tostring(L, -1));
		lua_pop(L, 2);  // remove the error object and the message handler
		return;
	}

	result = lua_pcall(L, 0, 0, -2);
	if(result) {
		Object::logStatic(
		    Object::LL_Error, "rzfilter_lua_module", "Cannot run lua file %s: %s\n", filename, lua_tostring(L, -1));
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
		                  "rzfilter_lua_module",
		                  "Lua register: onServerPacket must be a lua function matching its C counterpart: "
		                  "bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* "
		                  "packet, SessionType sessionType)\n");
	}
	lua_pop(L, 1);

	lua_getglobal(L, "onClientPacket");
	if(lua_isfunction(L, -1)) {
		lua_onClientPacketFunction = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);  // for the next pop as luaL_ref pop the value
	} else {
		lua_onClientPacketFunction = LUA_NOREF;
		Object::logStatic(Object::LL_Info,
		                  "rzfilter_lua_module",
		                  "Lua register: onClientPacket must be a lua function matching its C counterpart: "
		                  "bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* "
		                  "packet, SessionType sessionType)\n");
	}
	lua_pop(L, 1);

	lua_getglobal(L, "onUnknownPacket");
	if(lua_isfunction(L, -1)) {
		lua_onUnknownPacketFunction = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);  // for the next pop as luaL_ref pop the value
	} else {
		lua_onUnknownPacketFunction = LUA_NOREF;
		Object::logStatic(Object::LL_Info,
		                  "rzfilter_lua_module",
		                  "Lua register: onUnknownPacket must be a lua function matching its C counterpart: "
		                  "bool onUnknownPacket(IFilterEndpoint* fromEndpoint, IFilterEndpoint* toEndpoint, int "
		                  "packetId, SessionType sessionType, bool isServerPacket)\n");
	}
	lua_pop(L, 1);

	lua_getglobal(L, "onServerDisconnected");
	if(lua_isfunction(L, -1)) {
		lua_onServerDisconnectedFunction = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);  // for the next pop as luaL_ref pop the value
	} else {
		lua_onServerDisconnectedFunction = LUA_NOREF;
		Object::logStatic(Object::LL_Info,
		                  "rzfilter_lua_module",
		                  "Lua register: onServerDisconnected must be a lua function matching its C counterpart: "
		                  "bool onServerDisconnected(IFilterEndpoint* client, IFilterEndpoint* server)\n");
	}
	lua_pop(L, 1);

	lua_getglobal(L, "onClientDisconnected");
	if(lua_isfunction(L, -1)) {
		lua_onClientDisconnectedFunction = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);  // for the next pop as luaL_ref pop the value
	} else {
		lua_onClientDisconnectedFunction = LUA_NOREF;
		Object::logStatic(Object::LL_Info,
		                  "rzfilter_lua_module",
		                  "Lua register: onClientDisconnected must be a lua function matching its C counterpart: "
		                  "bool onClientDisconnected(IFilterEndpoint* client, IFilterEndpoint* server)\n");
	}
	lua_pop(L, 1);
}

void PacketFilter::deinitLuaVM() {
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
		if(lua_onServerDisconnectedFunction != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, lua_onServerDisconnectedFunction);
			lua_onServerDisconnectedFunction = LUA_NOREF;
		}
		if(lua_onClientDisconnectedFunction != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, lua_onClientDisconnectedFunction);
			lua_onClientDisconnectedFunction = LUA_NOREF;
		}
		lua_close(L);
		L = nullptr;
	}
}

int PacketFilter::luaPrintToLogs(lua_State* L) {
	int nargs = lua_gettop(L);

	for(int i = 1; i <= nargs; i++) {
		const char* str = lua_tostring(L, i);
		Object::logStatic(Object::LL_Info, "rzfilter_lua_module", "Lua message: %s\n", str);
	}

	return 0;
}

void PacketFilter::pushEndpoint(lua_State* L, LuaEndpointMetaTable* endpoint) {
	lua_pushlightuserdata(L, endpoint);
	luaL_getmetatable(L, LuaEndpointMetaTable::NAME);
	lua_setmetatable(L, -2);
}

bool PacketFilter::pushPacket(lua_State* L, const TS_MESSAGE* packet, int version, bool isServer) {
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

void PacketFilter::onCheckReload() {
	struct stat info;
	int ret = stat(LUA_MAIN_FILE, &info);
	if(ret == 0 && (info.st_mode & S_IFREG)) {
		// If different time as current and has not changed since previous check, do reload
		if(newLuaFileMtime != currentLuaFileMtime && newLuaFileMtime == info.st_mtime) {
			Object::logStatic(Object::LL_Info, "rzfilter_lua_module", "Reloading %s\n", LUA_MAIN_FILE);
			initLuaVM();
			currentLuaFileMtime = info.st_mtime;
		} else if(currentLuaFileMtime != info.st_mtime) {
			Object::logStatic(
			    Object::LL_Info, "rzfilter_lua_module", "%s changed, will reload next time\n", LUA_MAIN_FILE);
		}
		newLuaFileMtime = info.st_mtime;
	}
}

IFilter* createFilter(IFilterEndpoint* client, IFilterEndpoint* server, SessionType sessionType, IFilter* oldFilter) {
	Object::logStatic(Object::LL_Info, "rzfilter_lua_module", "Loaded filter from data: %p\n", oldFilter);
	return new PacketFilter(client, server, sessionType, (PacketFilter*) oldFilter);
}

void destroyFilter(IFilter* filter) {
	delete filter;
}
