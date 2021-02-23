// Copyright (C) 2021 by FranchuFranchu <fff999abc999@gmail.com>
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "LuaUtil.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <typeinfo>

using namespace LuaUtil;
using namespace std;
 

#include "LuaCppConversion.h"

lua_State *LuaUtil::L = luaL_newstate();
std::map<size_t, ClassDefinition> LuaUtil::definitions;
void LuaUtil::Initialize()
{
	luaL_openlibs(L);
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
