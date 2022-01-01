#pragma once

#include "Core/Object.h"
#include "Packet/EncodedInt.h"
#include "Packet/StructSerializer.h"
#include <algorithm>
#include <lua.hpp>
#include <sstream>

class LuaTableWriter : public StructSerializer {
private:
	lua_State* L;
	bool ok;

protected:
	void luaSet(const char* fieldName) {
		if(fieldName)
			lua_setfield(L, -2, fieldName);
	}

	void luaGet(const char* fieldName) {
		if(fieldName)
			lua_getfield(L, -1, fieldName);
	}

public:
	LuaTableWriter(lua_State* L, packet_version_t version) : StructSerializer(version) {
		this->L = L;
		ok = true;
	}

	void prepareWrite() {
		lua_createtable(L, 0, 0);
		ok = true;
	}

	bool getProcessStatus() { return ok; }

	uint32_t getParsedSize() const { return 0; }

	// Write functions /////////////////////////

	void writeHeader(uint32_t size, uint16_t id) {
		// written outside of this writer
	}

	// Primitives
	template<typename T>
	typename std::enable_if<is_primitive<T>::value && !std::is_floating_point<T>::value, void>::type write(
	    const char* fieldName, T val) {
		lua_pushinteger(L, val);
		luaSet(fieldName);
	}

	template<typename T>
	typename std::enable_if<is_primitive<T>::value && std::is_floating_point<T>::value, void>::type write(
	    const char* fieldName, T val) {
		lua_pushnumber(L, val);
		luaSet(fieldName);
	}

	// Encoded values
	template<typename T> void write(const char* fieldName, const EncodedInt<T>& val) {
		write<uint32_t>(fieldName, uint32_t(val));
	}

	// Objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type write(const char* fieldName, const T& val) {
		lua_createtable(L, 0, 0);

		val.serialize(this);

		luaSet(fieldName);
	}

	// String
	void writeString(const char* fieldName, const std::string& val, size_t maxSize) {
		lua_pushstring(L, val.c_str());
		luaSet(fieldName);
	}

	void writeDynString(const char* fieldName, const std::string& val, size_t count) {
		lua_pushstring(L, val.c_str());
		luaSet(fieldName);
	}

	void writeArray(const char* fieldName, const char* val, size_t size) {
		lua_pushstring(L, val);
		luaSet(fieldName);
	}

	// Fixed array
	template<typename T> void writeArray(const char* fieldName, const T* val, size_t size) {
		lua_createtable(L, (int) size, 0);

		for(size_t i = 0; i < size; i++) {
			write(nullptr, val[i]);
			lua_rawseti(L, -2, i + 1);
		}

		luaSet(fieldName);
	}

	// Fixed array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type writeArray(const char* fieldName,
	                                                                                   const U* val,
	                                                                                   size_t size) {
		writeArray<U>(fieldName, val, size);
	}

	// Dynamic array
	template<typename T> void writeDynArray(const char* fieldName, const std::vector<T>& val, uint32_t) {
		lua_createtable(L, (int) val.size(), 0);

		auto it = val.begin();
		auto itEnd = val.end();
		for(size_t i = 0; it != itEnd; ++it, ++i) {
			write(nullptr, *it);
			lua_rawseti(L, -2, i + 1);
		}

		luaSet(fieldName);
	}

	// Dynamic array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type writeDynArray(const char* fieldName,
	                                                                                      const std::vector<U>& val,
	                                                                                      uint32_t count) {
		writeDynArray<U>(fieldName, val, count);
	}

	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type writeSize(const char* fieldName, T size) {}

	void pad(const char* fieldName, size_t size) {}

	// Read functions /////////////////////////

	void readHeader(uint16_t& id) {}

	// Boolean
	template<typename T> void read(const char* fieldName, bool& val) {
		luaGet(fieldName);

		if(!lua_isinteger(L, -1)) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_module",
			                  "Bad field type for %s, expected integer, got %s\n",
			                  fieldName ? fieldName : "<null>",
			                  lua_typename(L, lua_type(L, -1)));
			ok = false;
		}

		val = lua_tointeger(L, -1) != 0;
		lua_pop(L, 1);
	}

	template<typename T, typename U>
	typename std::enable_if<is_primitive<U>::value && !std::is_floating_point<U>::value &&
	                            !std::is_same<U, bool>::value,
	                        void>::type
	read(const char* fieldName, U& val) {
		luaGet(fieldName);

		if(!lua_isinteger(L, -1)) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_module",
			                  "Bad field type for %s, expected integer, got %s\n",
			                  fieldName ? fieldName : "<null>",
			                  lua_typename(L, lua_type(L, -1)));
			ok = false;
		}

		if constexpr(is_strong_typed_primitive<T>::value) {
			val = static_cast<U>(static_cast<typename U::underlying_type>(lua_tointeger(L, -1)));
		} else {
			val = static_cast<U>(lua_tointeger(L, -1));
		}
		lua_pop(L, 1);
	}

	template<typename T, typename U>
	typename std::enable_if<is_primitive<U>::value && std::is_floating_point<U>::value, void>::type read(
	    const char* fieldName, U& val) {
		luaGet(fieldName);

		if(!lua_isnumber(L, -1)) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_module",
			                  "Bad field type for %s, expected number, got %s\n",
			                  fieldName ? fieldName : "<null>",
			                  lua_typename(L, lua_type(L, -1)));
			ok = false;
		}

		val = static_cast<U>(lua_tonumber(L, -1));
		lua_pop(L, 1);
	}

	// Encoded values
	template<typename T, typename U> void read(const char* fieldName, EncodedInt<U>& val) {
		uint32_t internalVal;
		read<uint32_t>(fieldName, internalVal);
		val = internalVal;
	}

	// Objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type read(const char* fieldName, T& val) {
		luaGet(fieldName);

		if(!lua_istable(L, -1)) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_module",
			                  "Bad field type for %s, expected table, got %s\n",
			                  fieldName ? fieldName : "<null>",
			                  lua_typename(L, lua_type(L, -1)));
			ok = false;
		} else {
			val.deserialize(this);
		}
		lua_pop(L, 1);
	}

	// String
	void readString(const char* fieldName, std::string& val, size_t size) {
		luaGet(fieldName);

		if(!lua_isstring(L, -1)) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_module",
			                  "Bad field type for %s, expected string, got %s\n",
			                  fieldName ? fieldName : "<null>",
			                  lua_typename(L, lua_type(L, -1)));
			ok = false;
		}

		const char* str = lua_tostring(L, -1);
		if(str)
			val.assign(str);
		lua_pop(L, 1);
	}

	void readDynString(const char* fieldName, std::string& val, size_t sizeToRead, bool hasNullTerminator) {
		readString(fieldName, val, sizeToRead);
	}
	void readEndString(const char* fieldName, std::string& val, bool hasNullTerminator) {
		readString(fieldName, val, 0);
	}

	// All fixed arrays
	template<typename T, typename U> void readArray(const char* fieldName, U* val, size_t size) {
		luaGet(fieldName);

		if(!lua_istable(L, -1)) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_module",
			                  "Bad field type for %s, expected array, got %s\n",
			                  fieldName ? fieldName : "<null>",
			                  lua_typename(L, lua_type(L, -1)));
			ok = false;
		} else {
			size_t arraySize = std::min(lua_rawlen(L, -1), size);
			for(size_t i = 0; i < arraySize; ++i) {
				lua_rawgeti(L, -1, i + 1);
				read<T>(nullptr, val[i]);
			}
		}

		lua_pop(L, 1);
	}

	// Dynamic array
	template<typename T> void readDynArray(const char* fieldName, std::vector<T>& val, uint32_t sizeToRead) {
		luaGet(fieldName);

		if(!lua_istable(L, -1)) {
			Object::logStatic(Object::LL_Error,
			                  "rzfilter_lua_module",
			                  "Bad field type for %s, expected array, got %s\n",
			                  fieldName ? fieldName : "<null>",
			                  lua_typename(L, lua_type(L, -1)));
			ok = false;
		} else {
			size_t arraySize = lua_rawlen(L, -1);

			val.resize(arraySize, T{});

			for(size_t i = 0; i < arraySize; ++i) {
				lua_rawgeti(L, -1, i + 1);
				read<T>(nullptr, val[i]);
			}
		}

		lua_pop(L, 1);
	}

	// End array, read to the end of stream
	template<typename T> void readEndArray(const char* fieldName, std::vector<T>& val) {
		readDynArray(fieldName, val, 0);
	}

	// read size for objects (std:: containers)
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type readSize(const char* fieldName, uint32_t& val) {}

	void discard(const char* fieldName, size_t size) {}
};
