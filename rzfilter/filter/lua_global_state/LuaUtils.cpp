#include "LuaUtils.h"
#include "Core/Utils.h"

const luaL_Reg LuaUtils::FUNCTIONS[] = {{"getTimeInMsec", &getTimeInMsec}, {NULL, NULL}};

int LuaUtils::initLua(lua_State* L) {
	luaL_newlib(L, FUNCTIONS);

	return 1;
}

int LuaUtils::getTimeInMsec(lua_State* L) {
	uint64_t time = Utils::getTimeInMsec();

	lua_pushinteger(L, time);
	return 1;
}

int LuaUtils::luaMessageHandler(lua_State* L) {
	const char* msg = lua_tostring(L, 1);
	luaL_traceback(L, L, msg, 1);
	return 1;
}
