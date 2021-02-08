#ifndef LUA_UTIL_STANDARD_TYPES_H_
#define LUA_UTIL_STANDARD_TYPES_H_

#include <functional>
#include <map>
#include <set>
#include <vector>

#include "LuaHeaders.h"

namespace LuaUtil
{
	class MapAttributeInstance
	{
	public:
		MapAttributeInstance() = default;
		
		MapAttributeInstance(
		std::function<void(void *mapPointer)> get,
		std::function<void(void *mapPointer)> set,
		std::function<void(void *mapPointer)> list);
		
		~MapAttributeInstance() = default;
		
		// These two functions have different behaviour depending on the mapPointer they were called on
		// The first upvalue is (void* mapPointer), which points to the map<> they were called on
		static int CFunction_index(lua_State *L);
		static int CFunction_newindex(lua_State *L);
		
		void CreateUserdata(lua_State *L, void *mapPointer) const;
	public:
		std::function<void(void *mapPointer)> get;
		std::function<void(void *mapPointer)> set;
		std::function<void(void *mapPointer)> list;
		
		size_t keyType = 0;
		size_t valueType = 0;
	};
};

#endif // LUA_UTIL_STANDARD_TYPES_H_
