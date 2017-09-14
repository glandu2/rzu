#include "PacketFilter.h"
#include "Core/Utils.h"
#include "LuaTableWriter.h"
#include "PacketTemplates.h"
#include "LuaTimer.h"

template<class T>
struct SendPacketFromLuaCallback {
	void operator()(lua_State* L, IFilterEndpoint* endpoint, bool* ok) {
		bool result = LuaEndpointMetaTable::sendPacketFromLua<T>(L, endpoint);
		if(ok)
			*ok = result;
	}
};

template<class T>
struct LuaDeclarePacketTypeNameCallback {
	void operator()(lua_State* L) {
		lua_pushinteger(L, T::getId(EPIC_LATEST));
		lua_setglobal(L, T::getName());
	}
};

template<class T>
struct LuaSerializePackerCallback {
	void operator()(lua_State* L, int version, const TS_MESSAGE* packet, bool* serializationSucess) {
		T deserializedPacket;
		MessageBuffer buffer((void*)packet, packet->size, version);

		deserializedPacket.deserialize(&buffer);
		if(buffer.checkPacketFinalSize()) {
			LuaTableWriter luaSerializer(L, version);
			luaSerializer.prepareWrite();
			deserializedPacket.serialize(&luaSerializer);
			lua_pushstring(L, T::getName());
			lua_setfield(L, -2, "__name");

			if(serializationSucess)
				*serializationSucess = true;
		} else {
			if(serializationSucess)
				*serializationSucess = false;
		}
	}
};

const char* const LuaEndpointMetaTable::NAME = "rzfilter.IFilterEndpoint";
const luaL_Reg LuaEndpointMetaTable::FUNCTIONS[] = {
    {"getPacketVersion", &getPacketVersion},
    {"sendAuthPacket", &sendAuthPacket},
    {"sendGamePacket", &sendGamePacket},
    {NULL, NULL}
};

int LuaEndpointMetaTable::getPacketVersion(lua_State* L) {
	LuaEndpointMetaTable *self = (LuaEndpointMetaTable*) luaL_checkudata(L, 1, NAME);
	if(!self || !self->endpoint)
		return 0;

	lua_pushinteger(L, self->endpoint->getPacketVersion());
	return 1;
}

int LuaEndpointMetaTable::sendAuthPacket(lua_State* L) {
	return sendPacket(L, [](int packetId, bool, lua_State* L, IFilterEndpoint* endpoint) {
		bool ok = true;
		processAuthPacket<SendPacketFromLuaCallback>(packetId, L, endpoint, &ok);
		return ok;
	});
}


int LuaEndpointMetaTable::sendGamePacket(lua_State* L) {
	return sendPacket(L, [](int packetId, bool isServerPacket, lua_State* L, IFilterEndpoint* endpoint) {
		bool ok = true;
		processGamePacket<SendPacketFromLuaCallback>(packetId, isServerPacket, L, endpoint, &ok);
		return ok;
	});
}

template<typename Callback>
int LuaEndpointMetaTable::sendPacket(lua_State* L, Callback callback) {

	LuaEndpointMetaTable *self = (LuaEndpointMetaTable*) luaL_checkudata(L, 1, NAME);

	if(!self || !self->endpoint)
		return 0;

	IFilterEndpoint *endpoint = self->endpoint;
	luaL_checktype(L, 2, LUA_TTABLE);

	lua_getfield(L, 2, "id");
	if(!lua_isinteger(L, -1)) {
		luaL_error(L, "Bad packet field type for id, expected integer, got %s", lua_typename(L, lua_type(L, -1)));
	}

	int packetId = (int)lua_tointeger(L, -1);
	lua_pop(L, 1);

	bool ok = callback(packetId, self->isServerEndpoint, L, endpoint);

	if(!ok) {
		luaL_error(L, "Couln't send packet %d", packetId);
	}

	return 0;
}

template<class T>
bool LuaEndpointMetaTable::sendPacketFromLua(lua_State* L, IFilterEndpoint *endpoint) {
	T packet;
	LuaTableWriter luaDeserialiser(L, endpoint->getPacketVersion());

	packet.deserialize(&luaDeserialiser);

	if(!luaDeserialiser.getProcessStatus())
		return false;

	endpoint->sendPacket(packet);

	return true;
}



PacketFilter::PacketFilter(IFilterEndpoint* client, IFilterEndpoint* server, PacketFilter *)
    : IFilter(client, server), clientEndpoint(client, false), serverEndpoint(server, true)
{
	initLuaVM();
}

PacketFilter::~PacketFilter()
{
	deinitLuaVM();
}

bool PacketFilter::onServerPacket(const TS_MESSAGE* packet, ServerType serverType)
{
	return luaCallPacket(lua_onServerPacketFunction, "onServerPacket", packet, serverType);
}

bool PacketFilter::onClientPacket(const TS_MESSAGE* packet, ServerType serverType)
{
	return luaCallPacket(lua_onClientPacketFunction, "onClientPacket", packet, serverType);
}

bool PacketFilter::luaCallPacket(int function, const char* functionName, const TS_MESSAGE* packet, ServerType serverType) {
	if(packet->id == 9999)
		return true;

	if(function != LUA_REFNIL && function != LUA_NOREF) {
		bool returnValue = true;

		int topBeforeCall = lua_gettop(L);

		lua_pushcfunction(L, &luaMessageHandler);

		lua_rawgeti(L, LUA_REGISTRYINDEX, function);
		pushEndpoint(L, &clientEndpoint);
		pushEndpoint(L, &serverEndpoint);
		if(!pushPacket(L, packet, server->getPacketVersion(), false, serverType)) {
			Object::logStatic(Object::LL_Error, "rzfilter_lua_module", "Cannot serialize packet id: %d\n",
			                             packet->id);
			lua_settop(L, topBeforeCall);
			return true;
		}
		lua_pushinteger(L, serverType);

		int result = lua_pcall(L, 4, 1, -6);
		if (result) {
			Object::logStatic(Object::LL_Error, "rzfilter_lua_module", "Cannot run lua %s function: %d:%s\n",
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

int PacketFilter::luaMessageHandler(lua_State* L) {
	const char *msg = lua_tostring(L, 1);
	luaL_traceback(L, L, msg, 1);
	return 1;
}

void PacketFilter::initLuaVM()
{
	deinitLuaVM();

	L = luaL_newstate();
	lua_onServerPacketFunction = LUA_NOREF;
	lua_onClientPacketFunction = LUA_NOREF;

	luaL_openlibs(L);

	// Override lua print function to output via the log system
	lua_pushcfunction(L, &PacketFilter::luaPrintToLogs);
	lua_setglobal(L, "print");

	iterateAllPacketTypes<LuaDeclarePacketTypeNameCallback>(L);

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

	lua_pushcfunction(L, &luaMessageHandler);

	const char* const filename = "rzfilter.lua";
	int result  = luaL_loadfilex(L, filename, nullptr);
	if (result) {
		Object::logStatic(Object::LL_Error, "rzfilter_lua_module", "Cannot load lua file %s: %s\n",
		                 filename,
		                 lua_tostring(L, -1));
		lua_pop(L, 2); // remove the error object and the message handler
		return;
	}

	result = lua_pcall(L, 0, 0, -2);
	if (result) {
		Object::logStatic(Object::LL_Error, "rzfilter_lua_module", "Cannot run lua file %s: %s\n",
		                 filename,
		                 lua_tostring(L, -1));
		lua_pop(L, 2); // remove the error object and the message handler
		return;
	}
	lua_pop(L, 1); // remove the message handler

	lua_getglobal(L, "onServerPacket");
	if(lua_isfunction(L, -1)) {
		lua_onServerPacketFunction = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushnil(L); // for the next pop as luaL_ref pop the value
	} else {
		Object::logStatic(Object::LL_Error, "rzfilter_lua_module",
		                  "Lua register: onServerPacket must be a lua function matching its C counterpart: "
		                  "bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType)\n");
	}
	lua_pop(L, 1);

	lua_getglobal(L, "onClientPacket");
	if(lua_isfunction(L, -1)) {
		lua_onClientPacketFunction = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushnil(L); // for the next pop as luaL_ref pop the value
	} else {
		Object::logStatic(Object::LL_Error, "rzfilter_lua_module",
		                  "Lua register: onClientPacket must be a lua function matching its C counterpart: "
		                  "bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType)\n");
	}
	lua_pop(L, 1);
}

void PacketFilter::deinitLuaVM()
{
	if(L) {
		if(lua_onServerPacketFunction != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, lua_onServerPacketFunction);
			lua_onServerPacketFunction = LUA_NOREF;
		}
		if(lua_onClientPacketFunction != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, lua_onClientPacketFunction);
			lua_onClientPacketFunction = LUA_NOREF;
		}
		lua_close(L);
		L = nullptr;
	}
}

int PacketFilter::luaPrintToLogs(lua_State* L)
{
	int nargs = lua_gettop(L);

	for (int i=1; i <= nargs; i++) {
		const char* str = lua_tostring(L, i);
		Object::logStatic(Object::LL_Info, "rzfilter_lua_module", "Lua message: %s\n", str);
	}

	return 0;
}

void PacketFilter::pushEndpoint(lua_State* L, LuaEndpointMetaTable* endpoint)
{
	lua_pushlightuserdata(L, endpoint);
	luaL_getmetatable(L, LuaEndpointMetaTable::NAME);
	lua_setmetatable(L, -2);
}

bool PacketFilter::pushPacket(lua_State* L, const TS_MESSAGE* packet, int version, bool isServer, ServerType serverType)
{
	bool ok;
	bool serializationSucess = false;

	if(serverType == ST_Game)
		ok = processGamePacket<LuaSerializePackerCallback>(packet->id, isServer, L, version, packet, &serializationSucess);
	else
		ok = processAuthPacket<LuaSerializePackerCallback>(packet->id, L, version, packet, &serializationSucess);

	return ok && serializationSucess;
}

IFilter *createFilter(IFilterEndpoint* client, IFilterEndpoint* server, IFilter *oldFilter)
{
	Object::logStatic(Object::LL_Info, "rzfilter_lua_module", "Loaded filter from data: %p\n", oldFilter);
	return new PacketFilter(client, server, (PacketFilter*)oldFilter);
}

void destroyFilter(IFilter *filter)
{
	delete filter;
}
