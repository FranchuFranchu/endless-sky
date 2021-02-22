// Generated from utils/generate_lua_integer_conversions.py
if (basicType == typeid(double).hash_code())
    lua_pushnumber(L, *reinterpret_cast<double*>(pointer));
else if (basicType == typeid(float).hash_code())
    lua_pushnumber(L, *reinterpret_cast<float*>(pointer));
else if (basicType == typeid(uint8_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<uint8_t*>(pointer));
else if (basicType == typeid(uint16_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<uint16_t*>(pointer));
else if (basicType == typeid(uint32_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<uint32_t*>(pointer));
else if (basicType == typeid(uint64_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<uint64_t*>(pointer));
else if (basicType == typeid(int8_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<int8_t*>(pointer));
else if (basicType == typeid(int16_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<int16_t*>(pointer));
else if (basicType == typeid(int32_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<int32_t*>(pointer));
else if (basicType == typeid(int64_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<int64_t*>(pointer));
