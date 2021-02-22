// Copyright (C) 2021 by FranchuFranchu <fff999abc999@gmail.com>
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "Vector.h"

#include <iostream>

using namespace std;

LuaUtil::VectorAttributeInstance::VectorAttributeInstance(
		std::function<void(void *vectorPointer)> get,
		std::function<void(void *vectorPointer)> set,
		std::function<void(void *vectorPointer)> size) :
	get(get), set(set), size(size)
{
	
}



void LuaUtil::VectorAttributeInstance::CreateUserdata(lua_State *L, void *vectorPointer) const
{
	*reinterpret_cast<const LuaUtil::VectorAttributeInstance**>(lua_newuserdata(L, sizeof(this))) = this;
	// Create the metatable
	luaL_newmetatable(L, "LuaUtil.VectorAttributeInstance");
	
	
	lua_pushstring(L, "__index");
	// Push upvalues
	lua_pushlightuserdata(L, vectorPointer);
	lua_pushcclosure(L, &LuaUtil::VectorAttributeInstance::CFunction_index, 1);
	lua_settable(L, -3);
	
	
	lua_pushstring(L, "__newindex");
	// Push upvalues
	lua_pushlightuserdata(L, vectorPointer);
	lua_pushcclosure(L, &LuaUtil::VectorAttributeInstance::CFunction_newindex, 1);
	lua_settable(L, -3);
	
	
	lua_setmetatable(L, -2);
	
};


int LuaUtil::VectorAttributeInstance::CFunction_index(lua_State *L)
{
	LuaUtil::VectorAttributeInstance *vectorData = *reinterpret_cast<LuaUtil::VectorAttributeInstance**>(luaL_checkudata(L, 1, "LuaUtil.VectorAttributeInstance"));
	luaL_checktype(L, lua_upvalueindex(1), LUA_TLIGHTUSERDATA);
	void *vectorPointer = lua_touserdata(L, lua_upvalueindex(1));
	vectorData->get(vectorPointer);
	return 1;
}



int LuaUtil::VectorAttributeInstance::CFunction_newindex(lua_State *L)
{
	LuaUtil::VectorAttributeInstance *vectorData = *reinterpret_cast<LuaUtil::VectorAttributeInstance**>(luaL_checkudata(L, 1, "LuaUtil.VectorAttributeInstance"));
	luaL_checktype(L, lua_upvalueindex(1), LUA_TLIGHTUSERDATA);
	void *vectorPointer = lua_touserdata(L, lua_upvalueindex(1));
	vectorData->set(vectorPointer);
	return 1;
}
