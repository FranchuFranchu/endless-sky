// Copyright (C) 2021 by FranchuFranchu <fff999abc999@gmail.com>
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef LUA_UTIL_H_
#define LUA_UTIL_H_

#include "LuaHeaders.h"
#include "stl/Map.h"
#include "stl/Vector.h"

#include <map>
#include <vector>

using namespace std;

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

namespace LuaUtil {
	
	extern lua_State *L;
	
	void Initialize();
	void ClassObjectToLua(std::string name, void *object);
	int CFunction_index(lua_State *L);
	
	void ObjectToLua(void *pointer, std::vector<size_t> type);
	void LuaToObject(void *pointer, std::vector<size_t> type);
	
	class ClassDefinition
	{
	public:
		template <typename RootType, typename AttributeType>
		LuaUtil::ClassDefinition& property(std::string name, AttributeType RootType::* memberPointer);
		
		template <typename RootType, typename ValueType>
		LuaUtil::ClassDefinition& property(std::string name, std::vector<ValueType> RootType::* memberPointer);
		
		template <typename RootType, typename KeyType, typename ValueType>
		LuaUtil::ClassDefinition& property(std::string name, std::map<KeyType, ValueType> RootType::* memberPointer);
		
		
		template <typename RootType>
		void save(std::string name);
		
		std::map<std::string, std::ptrdiff_t> propertyOffsets;
		// Here, the 0th element is the type, and subsequent elements are template parameters
		std::map<std::string, std::vector<size_t>> propertyTypes;
		
		std::map<std::string, LuaUtil::MapAttributeInstance> propertyMapManagers;
		std::map<std::string, LuaUtil::VectorAttributeInstance> propertyVectorManagers;
	};
	
	extern std::map<size_t, ClassDefinition> definitions;
} // LuaUtil


class Test2
{
public:
	Test2() = default;
	
	~Test2() = default;
	
	static void Initialize()
	{
		
		LuaUtil::definitions[typeid(Test2).hash_code()] = LuaUtil::ClassDefinition();
		LuaUtil::definitions[typeid(Test2).hash_code()]
			.property("attribute_one", &Test2::attributeOne)
			.property("attribute_two", &Test2::attributeTwo)
			.property("attribute_three", &Test2::attributeThree)
			.save<Test2>("esky.Test");
	};
	int attributeOne = 0;
	std::map<int, int> attributeTwo;
	std::vector<int> attributeThree;
};

template <typename RootType, typename AttributeType>
LuaUtil::ClassDefinition& LuaUtil::ClassDefinition::property(string name, AttributeType RootType::* memberPointer)
{
	propertyOffsets[name] = memberOffset(memberPointer);
	propertyTypes[name] = vector<size_t>{typeid(AttributeType).hash_code()};
	return *this;
}

template <typename RootType, typename ValueType>
LuaUtil::ClassDefinition& LuaUtil::ClassDefinition::property(std::string name, std::vector<ValueType> RootType::* memberPointer)
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
LuaUtil::ClassDefinition& LuaUtil::ClassDefinition::property(std::string name, std::map<KeyType, ValueType> RootType::* memberPointer)
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



#endif // LUA_UTIL_H_
