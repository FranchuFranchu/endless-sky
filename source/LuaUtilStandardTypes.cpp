#include "LuaUtilStandardTypes.h"

#include <iostream>

using namespace std;

LuaUtil::MapAttributeInstance::MapAttributeInstance(
		std::function<void(void *mapPointer)> get,
		std::function<void(void *mapPointer)> set,
		std::function<void(void *mapPointer)> list) :
	get(get), set(set), list(list)
{
	
}



void LuaUtil::MapAttributeInstance::CreateUserdata(lua_State *L, void *mapPointer) const
{
	*reinterpret_cast<const LuaUtil::MapAttributeInstance**>(lua_newuserdata(L, sizeof(this))) = this;
	// Create the metatable
	luaL_newmetatable(L, "LuaUtil.MapAttributeInstance");
	lua_pushstring(L, "__index");
	// Push upvalues
	printf("%lx\n", reinterpret_cast<unsigned long>(this));
	lua_pushlightuserdata(L, mapPointer);
	lua_pushcclosure(L, &LuaUtil::MapAttributeInstance::CFunction_index, 1);
	lua_settable(L, -3);
	
	lua_setmetatable(L, -2);
	
};


int LuaUtil::MapAttributeInstance::CFunction_index(lua_State *L)
{
	LuaUtil::MapAttributeInstance *mapData = *reinterpret_cast<LuaUtil::MapAttributeInstance**>(luaL_checkudata(L, 1, "LuaUtil.MapAttributeInstance"));
	luaL_checktype(L, lua_upvalueindex(1), LUA_TLIGHTUSERDATA);
	void *mapPointer = lua_touserdata(L, lua_upvalueindex(1));
	mapData->get(mapPointer);
	return 1;
}
