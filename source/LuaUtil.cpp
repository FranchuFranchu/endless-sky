#include "LuaUtil.h"
#include "LuaUtilStandardTypes.h"

#include <cstring>
#include <iostream>
#include <typeinfo>

using namespace LuaUtil;
using namespace std;

class Test
{
public:
	Test() = default;
	~Test() = default;
	static void Initialize()
	{
		(*new ClassDefinition())
			.property("attribute_one", &Test::attributeOne)
			.save<Test>("esky.Test");
	};
	int attributeOne;
	char s[48];
	//map<int, int> attributeTwo;
};

// Hacky trick to get the offset of a poniter-to-member
template<class T, typename U>
ptrdiff_t memberOffset(U T::* member)
{
    return reinterpret_cast<ptrdiff_t>(
        &(reinterpret_cast<T const volatile*>(NULL)->*member)
    );
}

lua_State *LuaUtil::L = luaL_newstate();
void LuaUtil::Initialize()
{
	luaL_openlibs(L);
	Test::Initialize();
	
	Test test = Test();
	test.attributeOne = 32;
	strncpy(test.s, "Lorem ipsum dolor sit amet, consectetur adipisc", 48);
	
	cout << test.s << endl;
	
	ClassObjectToLua("esky.Test", &test);
	lua_setglobal(L, "thing");
	
	int error = luaL_loadstring(L, "print(thing.attribute_one)")
		|| lua_pcall(L, 0, 0, 0);
	
	
	if(error)
	{
		cerr << "Lua error: " << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}
	
	cout << test.s << endl;
}



void LuaUtil::ClassObjectToLua(string name, void *object)
{
	*reinterpret_cast<void**>(lua_newuserdata(L, sizeof(void**))) = object;
	
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
		auto t = definition.propertyMapManagers.at(propertyName);
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

template <typename RootType, typename KeyType, typename ValueType>
ClassDefinition& LuaUtil::ClassDefinition::property(std::string name, std::map<KeyType, ValueType> RootType::* memberPointer)
{
	propertyOffsets[name] = memberOffset(memberPointer);
	propertyTypes[name] = vector<size_t>{typeid(map<bool, bool>).hash_code(), typeid(KeyType).hash_code(), typeid(ValueType).hash_code()};
	propertyMapManagers[name] = MapAttributeInstance(
		// Get function
		[](void *mapPointer) {
			lua_pop(L, 1);
			lua_pushstring(L, "Hello, world!");
		},
		nullptr,
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
	
	// Copy ourselves to a new userdata object and push it to the stack
	// Copying the map<> means that this class shouldn't be edited after this is done
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

#include "LuaUtilTypeConversion.h"
