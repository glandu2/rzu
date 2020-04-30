#include "LuaTimer.h"
#include "LuaUtils.h"
#include "ClassCounter.h"

DECLARE_CLASSCOUNT_STATIC(LuaTimer)

const char* const LuaTimer::NAME = "LuaTimer";
const luaL_Reg LuaTimer::FUNCTIONS[] = {{"start", &start},
                                        {"stop", &stop},
                                        {"again", &again},
                                        {"setRepeat", &setRepeat},
                                        {"getRepeat", &getRepeat},
                                        {NULL, NULL}};

int LuaTimer::initLua(lua_State* L) {
	luaL_newmetatable(L, NAME);

	lua_pushcfunction(L, &gc);
	lua_setfield(L, -2, "__gc");

	// set __index as a table with custom methods
	lua_pushstring(L, "__index");
	lua_newtable(L);
	luaL_setfuncs(L, FUNCTIONS, 0);
	lua_settable(L, -3);

	lua_pop(L, 1);

	lua_newtable(L);
	lua_pushcfunction(L, &createTimer);
	lua_setfield(L, -2, "new");

	return 1;
}

int LuaTimer::createTimer(lua_State* L) {
	LuaTimer* self = new LuaTimer();
	*static_cast<LuaTimer**>(lua_newuserdata(L, sizeof(LuaTimer*))) = self;

	luaL_getmetatable(L, NAME);
	lua_setmetatable(L, -2);

	return 1;
}

int LuaTimer::gc(lua_State* L) {
	LuaTimer** userData = static_cast<LuaTimer**>(luaL_checkudata(L, 1, NAME));
	if(!userData || !*userData)
		return 0;

	if((*userData)->callbackLuaFunction != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, (*userData)->callbackLuaFunction);
		(*userData)->callbackLuaFunction = LUA_NOREF;
	}

	delete *userData;
	*userData = nullptr;

	return 0;
}

LuaTimer* LuaTimer::check_userdata(lua_State* L, int idx) {
	LuaTimer** userData = static_cast<LuaTimer**>(luaL_checkudata(L, idx, NAME));
	if(!userData)
		return nullptr;

	return *userData;
}

int LuaTimer::start(lua_State* L) {
	LuaTimer* self = check_userdata(L, 1);
	if(!self)
		return 0;

	luaL_checktype(L, 2, LUA_TFUNCTION);

	uint64_t timeout = luaL_checkinteger(L, 3);
	uint64_t repeat = luaL_checkinteger(L, 4);

	self->L = L;

	int result = self->timer.start(self, &LuaTimer::onTimerCallback, timeout, repeat);

	// Reference self while the timer is running to avoid timer GC even if nothing directly reference the timer in lua
	// the callback is still a way to get the timer
	self->updateRef(L, 1);

	if(result < 0)
		return luaL_error(L, "%s: %s", uv_err_name(result), uv_strerror(result));

	if(self->callbackLuaFunction != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, self->callbackLuaFunction);
		self->callbackLuaFunction = LUA_NOREF;
	}
	lua_pushvalue(L, 2);
	self->callbackLuaFunction = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_pushinteger(L, result);
	return 1;
}

int LuaTimer::stop(lua_State* L) {
	LuaTimer* self = check_userdata(L, 1);
	if(!self)
		return 0;

	int result = self->timer.stop();

	self->updateRef(L, 1);

	if(result < 0)
		return luaL_error(L, "%s: %s", uv_err_name(result), uv_strerror(result));

	lua_pushinteger(L, result);
	return 1;
}

int LuaTimer::again(lua_State* L) {
	LuaTimer* self = check_userdata(L, 1);
	if(!self)
		return 0;

	int result = self->timer.again();

	self->updateRef(L, 1);

	if(result < 0)
		return luaL_error(L, "%s: %s", uv_err_name(result), uv_strerror(result));

	lua_pushinteger(L, result);
	return 1;
}

int LuaTimer::setRepeat(lua_State* L) {
	LuaTimer* self = check_userdata(L, 1);
	if(!self)
		return 0;

	uint64_t repeat = luaL_checkinteger(L, 2);

	self->timer.setRepeat(repeat);

	return 0;
}

int LuaTimer::getRepeat(lua_State* L) {
	LuaTimer* self = check_userdata(L, 1);
	if(!self)
		return 0;

	uint64_t result = self->timer.getRepeat();

	lua_pushinteger(L, result);
	return 1;
}

void LuaTimer::onTimerCallback() {
	if(L && callbackLuaFunction != LUA_NOREF) {
		if(luaUserDataRef == LUA_NOREF) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_global_state_module",
			                  "Cannot call lua timer callback: no ref to lua timer\n");
			return;
		}

		lua_pushcfunction(L, &LuaUtils::luaMessageHandler);
		lua_rawgeti(L, LUA_REGISTRYINDEX, callbackLuaFunction);
		lua_rawgeti(L, LUA_REGISTRYINDEX, luaUserDataRef);
		int result = lua_pcall(L, 1, 0, -3);

		// Update reference to self, if the timer does not run anymore, remove self ref
		lua_rawgeti(L, LUA_REGISTRYINDEX, luaUserDataRef);
		updateRef(L, -1);
		lua_pop(L, 1);

		if(result == LUA_ERRRUN) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_global_state_module",
			                  "Cannot call lua timer callback: %s\n",
			                  lua_tostring(L, -1));
			lua_pop(L, 2);  // remove the error object and the message handler
			return;
		}
		lua_pop(L, 1);  // remove the message handler
	} else {
		timer.stop();
	}
}

void LuaTimer::updateRef(lua_State* L, int idx) {
	if(!timer.isRunning()) {
		if(luaUserDataRef != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, luaUserDataRef);
			luaUserDataRef = LUA_NOREF;
		}
	} else {
		if(luaUserDataRef == LUA_NOREF) {
			lua_pushvalue(L, idx);
			luaUserDataRef = luaL_ref(L, LUA_REGISTRYINDEX);
		}
	}
}
