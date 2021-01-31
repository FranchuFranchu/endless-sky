#ifndef LUA_UTIL_H_
#define LUA_UTIL_H_

#include "LuaHeaders.h"
#include "LuaUtilStandardTypes.h"

#include <map>
#include <vector>

namespace LuaUtil {
	
	extern lua_State *L;
	
	void Initialize();
	void ClassObjectToLua(std::string name, void *object);
	int CFunction_index(lua_State *L);
	
	void ObjectToLua(void *pointer, std::vector<size_t> type);
	void *LuaToObject(int index, size_t type);	
	
	class ClassDefinition
	{
	public:
		template <typename RootType, typename AttributeType>
		ClassDefinition& property(std::string name, AttributeType RootType::* memberPointer);
		
		template <typename RootType, typename KeyType, typename ValueType>
		ClassDefinition& property(std::string name, std::map<KeyType, ValueType> RootType::* memberPointer);
		
		template <typename RootType>
		void save(std::string name);
		
		std::map<std::string, std::ptrdiff_t> propertyOffsets;
		// Here, the 0th element is the type, and subsequent elements are template parameters
		std::map<std::string, std::vector<size_t>> propertyTypes;
		
		std::map<std::string, LuaUtil::MapAttributeInstance> propertyMapManagers;
	};
} // LuaUtil

#endif // LUA_UTIL_H_
