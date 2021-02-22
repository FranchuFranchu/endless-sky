// Copyright (C) 2021 by FranchuFranchu <fff999abc999@gmail.com>
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef LUA_UTIL_STL_MAP_H_
#define LUA_UTIL_STL_MAP_H_

#include <functional>

#include "../LuaHeaders.h"

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

#endif // LUA_UTIL_STL_MAP_H_
