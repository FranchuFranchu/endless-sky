// Copyright (C) 2021 by FranchuFranchu <fff999abc999@gmail.com>
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "Set.h"
#include "common.h"

#include <iostream>

using namespace std;

LuaUtil::SetAttributeInstance::SetAttributeInstance(
		std::function<void(void *setPointer)> add,
		std::function<void(void *setPointer)> remove,
		std::function<void(void *setPointer)> has,
		std::function<void(void *setPointer)> table) :
	add(add), remove(remove), has(has), table(table)
{
	
}



void LuaUtil::SetAttributeInstance::CreateUserdata(lua_State *L, void *setPointer) const
{
	*reinterpret_cast<const LuaUtil::SetAttributeInstance**>(lua_newuserdata(L, sizeof(this))) = this;
	// Create the metatable
	luaL_newmetatable(L, "LuaUtil.SetAttributeInstance");
	
	lua_pushstring(L, "__index");
	
	// function_table = {}
	lua_newtable(L);
	
	// Create the functions
	lua_pushstring(L, "add");
	lua_pushlightuserdata(L, setPointer);
	lua_pushcclosure(L, &LuaUtil::SetAttributeInstance::CFunction_add, 1);
	lua_settable(L, -3);
	
	lua_pushstring(L, "remove");
	lua_pushlightuserdata(L, setPointer);
	lua_pushcclosure(L, &LuaUtil::SetAttributeInstance::CFunction_remove, 1);
	lua_settable(L, -3);
	
	lua_pushstring(L, "has");
	lua_pushlightuserdata(L, setPointer);
	lua_pushcclosure(L, &LuaUtil::SetAttributeInstance::CFunction_has, 1);
	lua_settable(L, -3);
	
	lua_pushstring(L, "table");
	lua_pushlightuserdata(L, setPointer);
	lua_pushcclosure(L, &LuaUtil::SetAttributeInstance::CFunction_table, 1);
	lua_settable(L, -3);
	
	// metatable.__index = function_table
	lua_settable(L, -3);
	
	lua_pushstring(L, "_pointer");
	lua_pushlightuserdata(L, setPointer);
	lua_settable(L, -3);
	
	lua_setmetatable(L, -2);
	
};

namespace {
	inline LuaUtil::SetAttributeInstance *getObjectData(lua_State *L)
	{
		LuaUtil::SetAttributeInstance *setData = *reinterpret_cast<LuaUtil::SetAttributeInstance**>(luaL_checkudata(L, 1, "LuaUtil.SetAttributeInstance"));
		return setData;
	}
}


int LuaUtil::SetAttributeInstance::CFunction_add(lua_State *L)
{
	getObjectData(L)->add(getObjectPointer(L));
	return 1;
}


int LuaUtil::SetAttributeInstance::CFunction_remove(lua_State *L)
{
	getObjectData(L)->remove(getObjectPointer(L));
	return 1;
}


int LuaUtil::SetAttributeInstance::CFunction_has(lua_State *L)
{
	getObjectData(L)->has(getObjectPointer(L));
	return 1;
}


int LuaUtil::SetAttributeInstance::CFunction_table(lua_State *L)
{
	getObjectData(L)->table(getObjectPointer(L));
	return 1;
}
