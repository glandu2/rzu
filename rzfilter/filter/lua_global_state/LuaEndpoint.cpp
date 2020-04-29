#include "LuaEndpoint.h"
#include "LuaTableWriter.h"
#include "PacketIterator.h"
#include "ClassCounter.h"

DECLARE_CLASSCOUNT_STATIC(LuaEndpoint)

const char* const LuaEndpoint::NAME = "rzfilter.IFilterEndpoint";
const luaL_Reg LuaEndpoint::FUNCTIONS[] = {{"getPacketVersion", &getPacketVersion},
                                           {"sendPacket", &sendPacket},
                                           {"close", &close},
                                           {"getIp", &getIp},
                                           {"banIp", &banIp},
                                           {NULL, NULL}};

template<class T> struct LuaEndpoint::SendPacketFromLuaCallback {
	bool operator()(lua_State* L, IFilterEndpoint* endpoint) { return LuaEndpoint::sendPacketFromLua<T>(L, endpoint); }
};

int LuaEndpoint::getPacketVersion(lua_State* L) {
	LuaEndpoint* self = check_userdata(L, 1);
	if(!self || !self->endpoint)
		return 0;

	lua_pushinteger(L, self->endpoint->getPacketVersion());
	return 1;
}

int LuaEndpoint::sendPacket(lua_State* L) {
	LuaEndpoint* self = check_userdata(L, 1);
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
	SessionType sessionType;
	SessionPacketOrigin origin;

	if(self->serverType == IFilter::ST_Auth)
		sessionType = SessionType::AuthClient;
	else
		sessionType = SessionType::GameClient;

	if(self->isServerEndpoint)
		origin = SessionPacketOrigin::Client;
	else
		origin = SessionPacketOrigin::Server;

	if(!processPacket<SendPacketFromLuaCallback>(sessionType, origin, packetId, ok, L, endpoint)) {
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

int LuaEndpoint::close(lua_State* L) {
	LuaEndpoint* self = check_userdata(L, 1);
	if(!self || !self->endpoint)
		return 0;

	self->endpoint->close();
	return 0;
}

int LuaEndpoint::getIp(lua_State* L) {
	LuaEndpoint* self = check_userdata(L, 1);
	if(!self || !self->endpoint)
		return 0;

	StreamAddress address = self->endpoint->getAddress();
	char ip[108];
	address.getName(ip, sizeof(ip));
	lua_pushstring(L, ip);
	return 1;
}

int LuaEndpoint::banIp(lua_State* L) {
	LuaEndpoint* self = check_userdata(L, 1);
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

template<class T> bool LuaEndpoint::sendPacketFromLua(lua_State* L, IFilterEndpoint* endpoint) {
	T packet;
	LuaTableWriter luaDeserialiser(L, endpoint->getPacketVersion());

	packet.deserialize(&luaDeserialiser);

	if(!luaDeserialiser.getProcessStatus())
		return false;

	endpoint->sendPacket(packet);

	return true;
}

LuaEndpoint* LuaEndpoint::createInstance(lua_State* L,
                                         IFilterEndpoint* endpoint,
                                         bool isServerEndpoint,
                                         IFilter::ServerType serverType) {
	LuaEndpoint* self = new LuaEndpoint(endpoint, isServerEndpoint, serverType);
	*static_cast<LuaEndpoint**>(lua_newuserdata(L, sizeof(LuaEndpoint*))) = self;

	luaL_getmetatable(L, NAME);
	lua_setmetatable(L, -2);

	return self;
}

LuaEndpoint* LuaEndpoint::check_userdata(lua_State* L, int idx) {
	LuaEndpoint** userData = static_cast<LuaEndpoint**>(luaL_checkudata(L, idx, NAME));
	if(!userData)
		return nullptr;

	return *userData;
}

void LuaEndpoint::detachEndpoint() {
	endpoint = nullptr;
}

void LuaEndpoint::initLua(lua_State* L) {
	// LuaEndpointMetaTable metatable wrapping IFilterEndpoint
	luaL_newmetatable(L, LuaEndpoint::NAME);

	lua_pushcfunction(L, &gc);
	lua_setfield(L, -2, "__gc");

	// set __index as a table with custom methods (like sendGamePacket)
	lua_pushstring(L, "__index");
	lua_newtable(L);
	luaL_setfuncs(L, LuaEndpoint::FUNCTIONS, 0);

	// Add a field to the __index table with key "data" with an empty table so the user can store anything
	lua_newtable(L);
	lua_setfield(L, -2, "data");

	lua_settable(L, -3);

	// pop metatable
	lua_pop(L, 1);
}

int LuaEndpoint::gc(lua_State* L) {
	LuaEndpoint** userData = static_cast<LuaEndpoint**>(luaL_checkudata(L, 1, NAME));
	if(!userData || !*userData)
		return 0;

	delete *userData;
	*userData = nullptr;

	return 0;
}

LuaEndpoint::~LuaEndpoint() {
	endpoint = nullptr;
}
