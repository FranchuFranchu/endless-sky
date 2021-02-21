// Included by LuaUtil.cpp

void LuaUtil::ObjectToLua(void *pointer, vector<size_t> type)
{
	size_t basicType = type.front();
	if (basicType == typeid(int).hash_code())
		lua_pushnumber(L, *reinterpret_cast<int*>(pointer));
	else
	{
		lua_pushstring(L, "No matching type!");
		lua_error(L);
	}
}


// Pops a Lua object frmo the stack
void LuaUtil::LuaToObject(void *pointer, vector<size_t> type)
{
	size_t basicType = type.front();
	printf("%d\n", lua_type(L, -3));
	printf("%d\n", lua_type(L, -2));
	printf("%d\n", lua_type(L, -1));
	if (lua_isnumber(L, -1))
	{
		if (basicType == typeid(int).hash_code())
		{
			*reinterpret_cast<int*>(pointer) = lua_tonumber(L, -1);
		}
		else 
		{
			lua_pushstring(L, "No matching type 2!");
			lua_error(L);
		}
		lua_pop(L, -1);
		return;
	}
	else
	{
		lua_pushstring(L, "No matching type 3!");
		lua_error(L);
	}
}
