// Generated from utils/generate_lua_integer_conversions.py
if (basicType == typeid(double).hash_code())
    lua_pushnumber(L, *reinterpret_cast<const double*>(pointer));
else if (basicType == typeid(float).hash_code())
    lua_pushnumber(L, *reinterpret_cast<const float*>(pointer));
else if (basicType == typeid(uint8_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<const uint8_t*>(pointer));
else if (basicType == typeid(uint16_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<const uint16_t*>(pointer));
else if (basicType == typeid(uint32_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<const uint32_t*>(pointer));
else if (basicType == typeid(uint64_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<const uint64_t*>(pointer));
else if (basicType == typeid(int8_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<const int8_t*>(pointer));
else if (basicType == typeid(int16_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<const int16_t*>(pointer));
else if (basicType == typeid(int32_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<const int32_t*>(pointer));
else if (basicType == typeid(int64_t).hash_code())
    lua_pushnumber(L, *reinterpret_cast<const int64_t*>(pointer));
