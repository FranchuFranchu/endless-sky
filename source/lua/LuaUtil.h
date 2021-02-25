// Copyright (C) 2021 by FranchuFranchu <fff999abc999@gmail.com>
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef LUA_UTIL_H_
#define LUA_UTIL_H_

#include "LuaHeaders.h"
#include "stl/Map.h"
#include "../Set.h"
#include "stl/Set.h"
#include "stl/Vector.h"

#include <map>
#include <iostream>
#include <set>
#include <vector>

using namespace std;

namespace {
	// Hacky trick to get the offset of a pointer-to-member
	template<class T, typename U>
	ptrdiff_t memberOffset(U T::* member)
	{
		return reinterpret_cast<ptrdiff_t>(
			&(reinterpret_cast<T const volatile*>(NULL)->*member)
		);
	}
}

template <class T, template <class...> class Template>
struct is_specialization : false_type {};

template <template <class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : true_type {};

template<class...> struct conjunction : true_type { };
template<class B1> struct conjunction<B1> : B1 { };
template<class B1, class... Bn>
struct conjunction<B1, Bn...> 
    : conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};
    
template <bool B>
using bool_constant = integral_constant<bool, B>;

namespace LuaUtil {
	
	extern lua_State *L;
	
	void Initialize();
	
	// Pushes an instance of registryName to the Lua stack
	void PushInstance(string registryName, const void *object);
	// Static objects can only use properties added with ClassDefinition::staticProperty
	void PushStatic(string registryName);
	
	int CFunction_index(lua_State *L);
	
	void ObjectToLua(const void *pointer, vector<size_t> type);
	void LuaToObject(void *pointer, vector<size_t> type);
	
	
	class ClassDefinition
	{
	public:
		template <typename T>
		LuaUtil::ClassDefinition& staticProperty(string name, T *objectPointer)
		{
			staticProperties[name] = objectPointer;
			propertyData<T, nullptr>(name);
			return *this;
		}
		
		template <typename T, typename RootType>
		LuaUtil::ClassDefinition& property(string name, T RootType::* memberPointer)
		{
			propertyOffsets[name] = memberOffset(memberPointer);
			propertyData<T, nullptr>(name);
			return *this;
		}
		
		template <typename RootType>
		void save(string name)
		{
			registryName = name;
			// The metatable
			lua_newtable(L);
			
			lua_pushstring(L, "__name");
			lua_pushstring(L, name.c_str());
			lua_settable(L, -3);
			
			// Store a pointer to ourselves
			// This object should never go out of scope to prevent the pointer from being invalid
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

	
		// The propertyData functions change the propertyTypes and the property managers depending on the type of the pointer
		// TODO: Change the enable_if magic to avoid using "nullptr"
		
		// For simple types such as int, double, etc.
		template <typename T, typename enable_if<
			conjunction<
				bool_constant<!bool(is_specialization<T, vector>{})>,
				bool_constant<!bool(is_specialization<T, map>{})>,
				bool_constant<!bool(is_specialization<T, Set>{})>,
				bool_constant<!bool(is_specialization<T, set>{})>
			>::value
		, void*>::type >
		void propertyData(string name)
		{
			propertyTypes[name] = vector<size_t>{typeid(T).hash_code()};
		}
		
		// For the vector<> class in the STL library.
		template <typename T, typename enable_if<is_specialization<T, vector>{}, void*>::type >
		void propertyData(string name)
		{
			using ValueType = typename T::value_type;
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
				[](void *vectorPointer) {
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
				// Table function
				[](void *vectorPointer) {
					vector<ValueType> &vectorData = *reinterpret_cast<vector<ValueType>*>(vectorPointer);
					size_t index = 1;
					
					lua_createtable(L, vectorData.size(), 0);
					for(ValueType &it : vectorData)
					{
						LuaUtil::LuaToObject(&it, {typeid(ValueType).hash_code()});
						LuaUtil::LuaToObject(&index,{typeid(size_t ).hash_code()});
						lua_settable(L, -3);
						++index;
					}
				}
			);
		}
		
		// For the map<> class in the STL library.
		template <typename T, typename enable_if<is_specialization<T, map>{}, void*>::type >
		void propertyData(string name)
		{
			using KeyType = typename T::key_type;
			using ValueType = typename T::mapped_type;
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
				// Table function
				[](void *mapPointer)
				{
					map<KeyType, ValueType> &mapData = *reinterpret_cast<map<KeyType, ValueType>*>(mapPointer);
					lua_newtable(L);
					for(auto &it : mapData)
					{
						LuaUtil::ObjectToLua(&it.first, {typeid(KeyType).hash_code()});
						LuaUtil::ObjectToLua(&it.second, {typeid(ValueType).hash_code()});
						lua_settable(L, -3);
					}
				}
			);
		}
		
		// For the set<> class in the STL library.
		template <typename T, typename enable_if<is_specialization<T, set>{}, void*>::type >
		void propertyData(string name)
		{
			using KeyType = typename T::key_type;
			propertyTypes[name] = vector<size_t>{typeid(set<bool>).hash_code(), typeid(KeyType).hash_code()};
			propertySetManagers[name] = SetAttributeInstance(
				// Add function
				nullptr,
				// Remove function
				nullptr,
				// Has function
				[](void *setPointer)
				{
					set<KeyType> &setData = *reinterpret_cast<set<KeyType>*>(setPointer);
					
					KeyType k;
					lua_pushvalue(L, 2);
					LuaToObject(&k, {typeid(KeyType).hash_code()});
					
					lua_pushboolean(L, setData.count(k));
				},
				// Table function
				[](void *setPointer)
				{
					set<KeyType> &setData = *reinterpret_cast<set<KeyType>*>(setPointer);
					lua_newtable(L);
					int index = 0;
					for(const KeyType &it : setData)
					{
						LuaUtil::ObjectToLua(&it, {typeid(KeyType).hash_code()});
						lua_pushnumber(L, index);
						lua_settable(L, -3);
						index++;
					}
				}
			);
		}
		
		// For the Set<> class in source/Set.h
		template <typename T, typename enable_if<is_specialization<T, Set>{}, void*>::type >
		void propertyData(string name)
		{
			using ValueType = typename T::value_type;
			propertyTypes[name] = vector<size_t>{typeid(Set<bool>).hash_code(), typeid(ValueType).hash_code()};
			propertyMapManagers[name] = MapAttributeInstance(
				// Get function
				[](void *setPointer) {
					Set<ValueType> &setData = *reinterpret_cast<Set<ValueType>*>(setPointer);
					string key;
					LuaUtil::LuaToObject(&key,   {typeid(string).hash_code()});
					if (!setData.Has(key))
						lua_pushnil(L);
					else
					{		
						ValueType &value = *setData.Get(key);
						LuaUtil::ObjectToLua(&value, {typeid(ValueType).hash_code()});
					}
				},
				// Set function
				[](void *setPointer) {
					Set<ValueType> &setData = *reinterpret_cast<Set<ValueType>*>(setPointer);
					string key;
					ValueType value;
					LuaUtil::LuaToObject(&value, {typeid(ValueType).hash_code()});
					LuaUtil::LuaToObject(&key,   {typeid(string   ).hash_code()});
					
					*setData.Get(key) = value;
				},
				// Table function
				[](void *setPointer)
				{
					Set<ValueType> &setData = *reinterpret_cast<Set<ValueType>*>(setPointer);
					lua_newtable(L);
					for(auto &it : setData)
					{
						LuaUtil::ObjectToLua(&it.first, {typeid(string).hash_code()});
						LuaUtil::ObjectToLua(&it.second, {typeid(ValueType).hash_code()});
						lua_settable(L, -3);
					}
				}
			);
		}
		
		map<string, void*> staticProperties;
		
		map<string, ptrdiff_t> propertyOffsets;
		// Here, the 0th element is the type, and subsequent elements are template parameters
		map<string, vector<size_t>> propertyTypes;
		
		map<string, LuaUtil::MapAttributeInstance> propertyMapManagers;
		map<string, LuaUtil::VectorAttributeInstance> propertyVectorManagers;
		map<string, LuaUtil::SetAttributeInstance> propertySetManagers;
		
		string registryName;
	};
	
	template <typename T>
	LuaUtil::ClassDefinition& AddDefinition();
	
	extern map<size_t, ClassDefinition> definitions;
} // LuaUtil



class Test2
{
public:
	Test2() = default;
	
	~Test2() = default;
	
	static void Initialize()
	{
		
		LuaUtil::AddDefinition<Test2>()
			.property("attribute_one", &Test2::attributeOne)
			.property("attribute_two", &Test2::attributeTwo)
			.property("attribute_three", &Test2::attributeThree)
			.save<Test2>("esky.Test");
	};
	int attributeOne = 0;
	map<int, int> attributeTwo;
	vector<int> attributeThree;
};



template <typename T>
LuaUtil::ClassDefinition& LuaUtil::AddDefinition()
{
	LuaUtil::definitions[typeid(T).hash_code()] = LuaUtil::ClassDefinition();
	return LuaUtil::definitions[typeid(T).hash_code()];
	
}




#endif // LUA_UTIL_H_
