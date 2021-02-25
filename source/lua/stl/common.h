#ifndef COMMON_H
#define COMMON_H

namespace {
	inline void *getObjectPointer(lua_State *L)
	{
		void *objPointer = lua_touserdata(L, lua_upvalueindex(1));
		luaL_checktype(L, lua_upvalueindex(1), LUA_TLIGHTUSERDATA);
		return objPointer;
	}
	inline void *getObjectPointerFromMetatable(lua_State *L)
	{
		lua_getmetatable(L, 1);
		lua_pushstring(L, "_pointer");
		lua_gettable(L, -2);
		void *objPointer = lua_touserdata(L, -1);
		// Pop the metatable
		lua_pop(L, 1); 
		return objPointer;
	}
}


#endif // COMMON_H
