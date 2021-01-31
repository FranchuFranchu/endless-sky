// Included by LuaUtil.cpp

void LuaUtil::ObjectToLua(void *pointer, vector<size_t> type)
{
	size_t basicType = type.front();
	if (basicType == typeid(int).hash_code())
	{
		lua_pushnumber(L, *reinterpret_cast<int*>(pointer));
	}
	else
	{
		lua_pushstring(L, "No matching type!");
		lua_error(L);
	}
}
