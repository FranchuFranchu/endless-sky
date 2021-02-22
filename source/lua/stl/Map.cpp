// Copyright (C) 2021 by FranchuFranchu <fff999abc999@gmail.com>
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "Map.h"

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
	lua_pushlightuserdata(L, mapPointer);
	lua_pushcclosure(L, &LuaUtil::MapAttributeInstance::CFunction_index, 1);
	lua_settable(L, -3);
	
	
	lua_pushstring(L, "__newindex");
	// Push upvalues
	lua_pushlightuserdata(L, mapPointer);
	lua_pushcclosure(L, &LuaUtil::MapAttributeInstance::CFunction_newindex, 1);
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



int LuaUtil::MapAttributeInstance::CFunction_newindex(lua_State *L)
{
	LuaUtil::MapAttributeInstance *mapData = *reinterpret_cast<LuaUtil::MapAttributeInstance**>(luaL_checkudata(L, 1, "LuaUtil.MapAttributeInstance"));
	luaL_checktype(L, lua_upvalueindex(1), LUA_TLIGHTUSERDATA);
	void *mapPointer = lua_touserdata(L, lua_upvalueindex(1));
	mapData->set(mapPointer);
	return 1;
}
