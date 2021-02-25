// Copyright (C) 2021 by FranchuFranchu <fff999abc999@gmail.com>
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef LUA_UTIL_STL_SET_H_
#define LUA_UTIL_STL_SET_H_

#include <functional>

#include "../LuaHeaders.h"

namespace LuaUtil
{
	class SetAttributeInstance
	{
	public:
		SetAttributeInstance() = default;
		
		SetAttributeInstance(
		std::function<void(void *setPointer)> add,
		std::function<void(void *setPointer)> remove,
		std::function<void(void *setPointer)> has,
		std::function<void(void *setPointer)> table);
		
		~SetAttributeInstance() = default;
		
		// These functions have different behaviour depending on the setPointer they were called on
		// The first upvalue is (void* setPointer), which points to the set<> they were called on
		static int CFunction_add(lua_State *L);
		static int CFunction_has(lua_State *L);
		static int CFunction_remove(lua_State *L);
		static int CFunction_table(lua_State *L);
		
		static int CFunction_pairs(lua_State *L);
		static int CFunction_next(lua_State *L);
		
		void CreateUserdata(lua_State *L, void *setPointer) const;
	public:
		std::function<void(void *setPointer)> add;
		std::function<void(void *setPointer)> remove;
		std::function<void(void *setPointer)> has;
		std::function<void(void *setPointer)> table;
		
		size_t keyType = 0;
		size_t valueType = 0;
	};
};

#endif // LUA_UTIL_STL_SET_H_
