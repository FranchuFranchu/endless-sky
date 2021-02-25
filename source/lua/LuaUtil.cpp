// Copyright (C) 2021 by FranchuFranchu <fff999abc999@gmail.com>
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "LuaUtil.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <typeinfo>
#include <stdexcept>

using namespace LuaUtil;
using namespace std;
 

#include "LuaCppConversion.h"

namespace {
	// The unmodified CFunctions
	lua_CFunction originalPairs = nullptr;
	lua_CFunction originalIpairs = nullptr;
	
	
	// We will override the default "pairs" and "ipairs" function to change their behaviour
	// They will allow us to iterate over our own STL wrappers (i.e. vector, map, etc.)
	// To do this, we'll convert them to tables and then use the original pairs function
	// The 1st upvalue here is the function to call internally: either originalPairs or originalIpairs
	// The reason why they're not separate functions is to avoid code repetition.
	int CFunction_pairs_or_ipairs(lua_State *L)
	{
		if (!lua_isuserdata(L, 1))
			return originalPairs(L);
		
		lua_getmetatable(L, 1);
		
		// Note: For Lua 5.1, the name is stored differently.
		lua_pushstring(L, "__name");
		lua_gettable(L, -2);
		if (!lua_isstring(L, -1))
			// This userdata has no name.
			luaL_error(L, "invalid argument #1 to pairs (table or named userdata expected, got nameless userdata)");
		
		string registryName(lua_tostring(L, -1));
		
		// Pop the metatable
		lua_pop(L, 1);
		
		lua_CFunction tableFunction = nullptr;
		
		if (registryName == "LuaUtil.SetAttributeInstance")
			tableFunction = LuaUtil::SetAttributeInstance::CFunction_table;
		else if (registryName == "LuaUtil.MapAttributeInstance")
			tableFunction = LuaUtil::MapAttributeInstance::CFunction_table;
		else if (registryName == "LuaUtil.VectorAttributeInstance")
			tableFunction = LuaUtil::VectorAttributeInstance::CFunction_table;
		
		if (!tableFunction)
		{
			luaL_error(L, "invalid argument #1 to pairs (userdata %s can not be iterated over)", registryName.c_str());
		}
		
		
		// We call the function to get the table
		// Argument 1 is the userdata, which is passed over to this function
		lua_pushcfunction(L, tableFunction);
		lua_pushvalue(L, 1);
		lua_call(L, 1, 1);
		
		int stackSize = lua_gettop(L);
		// Call the original pairs() function with our table
		lua_pushvalue(L, lua_upvalueindex(1));
		lua_pushvalue(L, -2);
		lua_call(L, 1, LUA_MULTRET);
		
		return lua_gettop(L) - stackSize;
	}
	
	
	
	int CFunction_table_call_internal(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TUSERDATA);
		
		lua_pushvalue(L, 1);
		
		lua_getmetatable(L, -1);
		
		// Note: For Lua 5.1, the name is stored differently.
		lua_pushstring(L, "__name");
		lua_gettable(L, -2);
		if (!lua_isstring(L, -1))
			// This userdata has no name.
			luaL_error(L, "invalid argument #1 to table (named userdata expected, got nameless userdata)");
		
		string registryName(lua_tostring(L, -1));
		
		// Pop the metatable
		lua_pop(L, 1);
		// Pop the copied userdata
		lua_pop(L, 1);
		
		if (registryName == "LuaUtil.SetAttributeInstance")
			return LuaUtil::SetAttributeInstance::CFunction_table(L);
		else if (registryName == "LuaUtil.MapAttributeInstance")
			return LuaUtil::MapAttributeInstance::CFunction_table(L);
		/*else if (registryName == "LuaUtil.VectorAttributeInstance")
			return LuaUtil::VectorAttributeInstance::CFunction_table(L);*/
		
		return 0;
	}
	
	int CFunction_table_call(lua_State *L)
	{
		int stackSize = lua_gettop(L);
		lua_pushcfunction(L, CFunction_table_call_internal);
		lua_pushvalue(L, 2);
		lua_call(L, 1, LUA_MULTRET);
		return lua_gettop(L) - stackSize;
	}
	
	
}

lua_State *LuaUtil::L = luaL_newstate();
std::map<size_t, ClassDefinition> LuaUtil::definitions;
void LuaUtil::Initialize()
{
	luaL_openlibs(L);
	
	// Override the table(), pairs(), and ipairs() functions
	lua_pushglobaltable(L);
	
	lua_getglobal(L, "pairs");
	originalPairs = lua_tocfunction(L, -1);
	lua_pop(L, 1);
	
	lua_getglobal(L, "ipairs");
	originalIpairs = lua_tocfunction(L, -1);
	lua_pop(L, 1);
	
	
	lua_pushcfunction(L, originalPairs);
	lua_pushcclosure(L, CFunction_pairs_or_ipairs, 1);
	lua_setglobal(L, "pairs");
	
	lua_pushcfunction(L, originalIpairs);
	lua_pushcclosure(L, CFunction_pairs_or_ipairs, 1);
	lua_setglobal(L, "ipairs");
	
	// This is the equialent of the following Lua code:
	// getmetatable(table)._call = CFunction_table_call
	lua_getglobal(L, "table");
	// If there's no metatable yet, create it.
	if (!lua_getmetatable(L, -1))
	{
		lua_newtable(L); 
		// Shallow copy the metatable
		lua_pushvalue(L, -1); 
		lua_setmetatable(L, -3);
	}
	lua_pushstring(L, "__call");
	lua_pushcfunction(L, CFunction_table_call);
	lua_settable(L, -3);

	lua_setmetatable(L, -2);

	lua_pop(L, 1); // Pop the metatable
	lua_pop(L, 1); // Pop the table
	
	if (!(originalPairs && originalIpairs))
	{
		throw runtime_error("Either pairs or ipairs were not cfunctions! (your version of the Lua library might be weird)");
	}
	
	Test2::Initialize();
	Test2 test = Test2();
	test.attributeOne = 5;
	test.attributeTwo[4] = 1;
	PushInstance("esky.Test", &test);
	lua_setglobal(L, "thing");
	
	int error = luaL_loadstring(L, "thing.attribute_three[1] = 2\nprint(thing.attribute_three[1])")
		|| lua_pcall(L, 0, 0, 0);
	
	if(error)
	{
		cerr << "Lua error: " << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}
}



void LuaUtil::PushInstance(string registryName, const void *object)
{
	// TODO saying that this is const is a lie, Lua may modify it at any moment
	// We should consider making actually const objects in the future.
	const void **ptr = reinterpret_cast<const void**>(lua_newuserdata(L, sizeof(void**)));
	*ptr = object;
	
	lua_pushstring(L, registryName.c_str());
	lua_gettable(L, LUA_REGISTRYINDEX);	
	lua_setmetatable(L, -2);
}



void LuaUtil::PushStatic(string registryName)
{
	void **ptr = reinterpret_cast<void**>(lua_newuserdata(L, sizeof(void**)));
	*ptr = nullptr;
	
	lua_pushstring(L, registryName.c_str());
	lua_gettable(L, LUA_REGISTRYINDEX);	
	lua_setmetatable(L, -2);
}



int LuaUtil::CFunction_index(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	string propertyName(luaL_checkstring(L, 2));
	void **object = reinterpret_cast<void**>(lua_touserdata(L, 1));
	
	lua_getmetatable(L, 1);
	
	lua_pushstring(L, "class_definition");
	lua_gettable(L, -2);
	ClassDefinition &definition = **reinterpret_cast<ClassDefinition**>(lua_touserdata(L, -1));
	
	// Pop the metatable (to balance the stack)
	lua_pop(L, 1);
	
	vector<size_t> objectType = definition.propertyTypes[propertyName];
	
	void *attributeObject;
	
	const bool invokedOnObject = *object != nullptr;
	
	// If there's an entry in propertyOffsets and it was invoked on an object, then it's a non-static property
	// If there's an entry in staticProperties, then it's a static property
	// Else, error out.
	if (invokedOnObject && definition.propertyOffsets.find(propertyName) != definition.propertyOffsets.end() )
	{
		ptrdiff_t difference = definition.propertyOffsets[propertyName];
		attributeObject = reinterpret_cast<void*>(reinterpret_cast<char*>(*object)+difference);
	}
	else if (definition.staticProperties.find(propertyName) != definition.staticProperties.end())
		attributeObject = definition.staticProperties[propertyName];
	else if (!invokedOnObject && definition.propertyOffsets.find(propertyName) != definition.propertyOffsets.end())
		luaL_error(L, "Attempt to access non-static property %s on static object!", propertyName.c_str());
	else
		luaL_error(L, "Property %s not found.", propertyName.c_str());
	
	if (typeid(map<bool, bool>).hash_code() == objectType.front() || typeid(Set<bool>).hash_code() == objectType.front())
	{
		const MapAttributeInstance &t = definition.propertyMapManagers.at(propertyName);
		t.CreateUserdata(L, attributeObject);
		return 1;
	}
	else if (typeid(vector<bool>).hash_code() == objectType.front() )
	{
		const VectorAttributeInstance &t = definition.propertyVectorManagers.at(propertyName);
		t.CreateUserdata(L, attributeObject);
		return 1;
	}
	else if (typeid(set<bool>).hash_code() == objectType.front())
	{
		const SetAttributeInstance &t = definition.propertySetManagers.at(propertyName);
		t.CreateUserdata(L, attributeObject);
		return 1;	
	}
	else
	{
		ObjectToLua(attributeObject, objectType);	
	}
	
	
	return 1;
}
