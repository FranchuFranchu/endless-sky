#ifndef LUA_UTIL_STANDARD_TYPES_H_
#define LUA_UTIL_STANDARD_TYPES_H_

#include <map>
#include <vector>
#include <set>

#include "LuaHeaders.h"

namespace LuaUtil
{
	class MapAttributeInstance
	{
	public:
		MapAttributeInstance() = default;
		MapAttributeInstance(void(*get)(void *mapPointer), void(*set)(void *mapPointer), void(*list)(void *mapPointer));
		
		// These two functions have different behaviour depending on the mapPointer they were called on
		// The first upvalue is (void* mapPointer), which points to the map<> they were called on
		static int CFunction_index(lua_State *L);
		static int CFunction_newindex(lua_State *L);
		
		void CreateUserdata(lua_State *L, void *mapPointer);
	private:
		void(*get)(void *mapPointer) = nullptr;
		void(*set)(void *mapPointer) = nullptr;
		void(*list)(void *mapPointer) = nullptr;
		
		size_t keyType = 0;
		size_t valueType = 0;
	};
};

#endif // LUA_UTIL_STANDARD_TYPES_H_
