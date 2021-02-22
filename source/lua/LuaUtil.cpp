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

namespace {
	// Hacky trick to get the offset of a poniter-to-member
	template<class T, typename U>
	ptrdiff_t memberOffset(U T::* member)
	{
		return reinterpret_cast<ptrdiff_t>(
			&(reinterpret_cast<T const volatile*>(NULL)->*member)
		);
	}
}

lua_State *LuaUtil::L = luaL_newstate();
std::map<size_t, ClassDefinition> LuaUtil::definitions;
void LuaUtil::Initialize()
{
	luaL_openlibs(L);
	Test2::Initialize();
	Test2 test = Test2();
	test.attributeOne = 5;	
	test.attributeTwo[4] = 1;
	ClassObjectToLua("esky.Test", &test);
	lua_setglobal(L, "thing");
	
	int error = luaL_loadstring(L, "thing.attribute_three[1] = 2\nprint(thing.attribute_three[1])")
		|| lua_pcall(L, 0, 0, 0);
	
	if(error)
	{
		cerr << "Lua error: " << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}
	
	printf("%d", test.attributeTwo[4]);
}



void LuaUtil::ClassObjectToLua(string name, void *object)
{
	void **ptr = reinterpret_cast<void**>(lua_newuserdata(L, 10));
	cout << hex << ptr << hex << endl;
	*ptr = object;
	
	lua_pushstring(L, name.c_str());
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
	
	ptrdiff_t difference = definition.propertyOffsets[propertyName];
	vector<size_t> objectType = definition.propertyTypes[propertyName];
	void *attributeObject = reinterpret_cast<void*>(reinterpret_cast<char*>(*object)+difference);
	if (typeid(map<bool, bool>).hash_code() == objectType.front())
	{
		const MapAttributeInstance &t = definition.propertyMapManagers.at(propertyName);
		t.CreateUserdata(L, attributeObject);
		return 1;
	}
	else if (typeid(vector<bool>).hash_code() == objectType.front())
	{
		const VectorAttributeInstance &t = definition.propertyVectorManagers.at(propertyName);
		t.CreateUserdata(L, attributeObject);
		return 1;
	}
	else
	{
		ObjectToLua(attributeObject, objectType);	
	}
	
	
	return 1;
}



template <typename RootType, typename AttributeType>
ClassDefinition& LuaUtil::ClassDefinition::property(string name, AttributeType RootType::* memberPointer)
{
	propertyOffsets[name] = memberOffset(memberPointer);
	propertyTypes[name] = vector<size_t>{typeid(AttributeType).hash_code()};
	return *this;
}

template <typename RootType, typename ValueType>
ClassDefinition& LuaUtil::ClassDefinition::property(std::string name, std::vector<ValueType> RootType::* memberPointer)
{
	propertyOffsets[name] = memberOffset(memberPointer);
	propertyTypes[name] = vector<size_t>{typeid(vector<bool>).hash_code(), typeid(ValueType).hash_code()};
	propertyVectorManagers[name] = VectorAttributeInstance(
		// Get function
		[](void *vectorPointer) {
			vector<ValueType> &vectorData = *reinterpret_cast<vector<ValueType>*>(vectorPointer);
			size_t key;
			LuaUtil::LuaToObject(&key,   {typeid(size_t ).hash_code()});
			key -= 1;
			if (key >= vectorData.size())
			{
				lua_pushstring(L, string("Index " + to_string(key + 1) + " out of range").c_str());
				lua_error(L);
			}
			
			ValueType value = vectorData[key];
			LuaUtil::ObjectToLua(&value, {typeid(ValueType).hash_code()});
		},
		// Set function
		[name](void *vectorPointer) {
			vector<ValueType> &vectorData = *reinterpret_cast<vector<ValueType>*>(vectorPointer);
			size_t key;
			ValueType value;
			LuaUtil::LuaToObject(&value, {typeid(ValueType).hash_code()});
			LuaUtil::LuaToObject(&key,   {typeid(size_t ).hash_code()});
			key -= 1;
			if ((key - vectorData.size()) > 1)
			{
				lua_pushstring(L, string("Index " + to_string(key + 1) + " out of range").c_str());
				lua_error(L);
			}
			else if (key == vectorData.size())
			{
				vectorData.push_back(value);
			}
			else 
				vectorData[key] = value;
		},
		nullptr
	);
	return *this;
	
}
		
template <typename RootType, typename KeyType, typename ValueType>
ClassDefinition& LuaUtil::ClassDefinition::property(std::string name, std::map<KeyType, ValueType> RootType::* memberPointer)
{
	propertyOffsets[name] = memberOffset(memberPointer);
	propertyTypes[name] = vector<size_t>{typeid(map<bool, bool>).hash_code(), typeid(KeyType).hash_code(), typeid(ValueType).hash_code()};
	propertyMapManagers[name] = MapAttributeInstance(
		// Get function
		[](void *mapPointer) {
			map<KeyType, ValueType> &mapData = *reinterpret_cast<map<KeyType, ValueType>*>(mapPointer);
			KeyType key;
			LuaUtil::LuaToObject(&key,   {typeid(KeyType).hash_code()});
			ValueType value = mapData[key];
			LuaUtil::ObjectToLua(&value, {typeid(ValueType).hash_code()});
		},
		// Set function
		[](void *mapPointer) {
			map<KeyType, ValueType> &mapData = *reinterpret_cast<map<KeyType, ValueType>*>(mapPointer);
			KeyType key;
			ValueType value;
			LuaUtil::LuaToObject(&value, {typeid(ValueType).hash_code()});
			LuaUtil::LuaToObject(&key,   {typeid(KeyType  ).hash_code()});
			
			mapData[key] = value;
		},
		nullptr
	);
	return *this;
}
		

template <typename RootType>
void LuaUtil::ClassDefinition::save(string name)
{
	// The metatable
	lua_newtable(L);
	
	lua_pushstring(L, "__name");
	lua_pushstring(L, name.c_str());
	lua_settable(L, -3);
	
	// Store a pointer to ourselves
	// This class should never go out of scope
	lua_pushstring(L, "class_definition");
	ClassDefinition **ptr = reinterpret_cast<ClassDefinition**>(lua_newuserdata(L, sizeof(this)));
	*ptr = this;
	lua_settable(L, -3);
	
	// Create the __index metamethod
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2); 
	lua_pushcclosure(L, &LuaUtil::CFunction_index, 1);
	lua_settable(L, -3);
	
	// This is equivalent to REGISTRY[index] = stack[-2] (which is the metatable we created)
	lua_pushstring(L, name.c_str());
	lua_pushvalue(L, -2); 
	lua_settable(L, LUA_REGISTRYINDEX);
}

