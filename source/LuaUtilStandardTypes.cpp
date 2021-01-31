#include "LuaUtilStandardTypes.h"

LuaUtil::MapAttributeInstance::MapAttributeInstance(
	void(*get)(void *mapPointer), void(*set)(void *mapPointer), void(*list)(void *mapPointer)) :
	get(get), set(set), list(list)
{
	
}



void LuaUtil::MapAttributeInstance::CreateUserdata(lua_State *L, void *mapPointer)
{
	return;
	*reinterpret_cast<LuaUtil::MapAttributeInstance**>(lua_newuserdata(L, sizeof(this))) = this;
	// Create the metatable
	luaL_newmetatable(L, "LuaUtil.MapAttributeInstance");
	lua_pushstring(L, "__index");
	// Push upvalues
	lua_pushlightuserdata(L, mapPointer);
	lua_pushcclosure(L, &LuaUtil::MapAttributeInstance::CFunction_index, 1);
	lua_settable(L, -3);
	
	// -2 is the userdata here
	// -1 is the metatable
	lua_pushvalue(L, -2);
	// Now they're shifted by -1 and the userdata is on top
	lua_setmetatable(L, -2);
	// Pop both the copied userdata and the metatable
	lua_pop(L, 2);
	
};


int LuaUtil::MapAttributeInstance::CFunction_index(lua_State *L)
{
	LuaUtil::MapAttributeInstance *mapData = *reinterpret_cast<LuaUtil::MapAttributeInstance**>(luaL_checkudata(L, 1, "LuaUtil.MapAttributeInstance"));
	luaL_checktype(L, lua_upvalueindex(1), LUA_TLIGHTUSERDATA);
	mapData->get(lua_touserdata(L, lua_upvalueindex(1)));
	return 1;
}
