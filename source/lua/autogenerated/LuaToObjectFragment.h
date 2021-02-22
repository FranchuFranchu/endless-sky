// Generated from utils/generate_lua_integer_conversions.py
if (basicType == typeid(double).hash_code())
    *reinterpret_cast<double*>(pointer) = lua_tonumber(L, -1);
else if (basicType == typeid(float).hash_code())
    *reinterpret_cast<float*>(pointer) = lua_tonumber(L, -1);
else if (basicType == typeid(uint8_t).hash_code())
    *reinterpret_cast<uint8_t*>(pointer) = lua_tonumber(L, -1);
else if (basicType == typeid(uint16_t).hash_code())
    *reinterpret_cast<uint16_t*>(pointer) = lua_tonumber(L, -1);
else if (basicType == typeid(uint32_t).hash_code())
    *reinterpret_cast<uint32_t*>(pointer) = lua_tonumber(L, -1);
else if (basicType == typeid(uint64_t).hash_code())
    *reinterpret_cast<uint64_t*>(pointer) = lua_tonumber(L, -1);
else if (basicType == typeid(int8_t).hash_code())
    *reinterpret_cast<int8_t*>(pointer) = lua_tonumber(L, -1);
else if (basicType == typeid(int16_t).hash_code())
    *reinterpret_cast<int16_t*>(pointer) = lua_tonumber(L, -1);
else if (basicType == typeid(int32_t).hash_code())
    *reinterpret_cast<int32_t*>(pointer) = lua_tonumber(L, -1);
else if (basicType == typeid(int64_t).hash_code())
    *reinterpret_cast<int64_t*>(pointer) = lua_tonumber(L, -1);
