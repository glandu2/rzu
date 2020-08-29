#pragma once

#include "Core/Utils.h"
#include "HandleGenerator.h"
#include "Packet/EncodedInt.h"
#include "Packet/GameTypes.h"
#include "Packet/StructSerializer.h"
#include <unordered_map>

template<class HandleConverterLambda> class PacketHandleConverter : public StructSerializer {
private:
	HandleConverterLambda& handleConverterLambda;

public:
	PacketHandleConverter(packet_version_t version, HandleConverterLambda& handleConverterLambda)
	    : StructSerializer(version), handleConverterLambda(handleConverterLambda) {}

	uint32_t getParsedSize() const { return 0; }

	// Read ar_handle_t primitive, we are handling only these

	// ar_handle_t primitive, we want this
	template<typename T, typename U>
	std::enable_if_t<std::is_same<ar_handle_t, U>::value, void> read(const char* fieldName, U& val) {
		handleConverterLambda(fieldName, val);
	}

	//	// Write functions /////////////////////////

	//	void writeHeader(uint32_t /* size */, uint16_t id) { write<uint16_t>("id", id); }

	//	// handle ambiguous << operator for int16_t and int8_t
	//	template<typename T>
	//	typename std::enable_if<is_primitive<T>::value && sizeof(T) < sizeof(int), void>::type write(const char*
	// fieldName, 	                                                                                             T val)
	// {}

	//	// Primitives
	//	template<typename T>
	//	typename std::enable_if<is_primitive<T>::value && sizeof(T) >= sizeof(int), void>::type write(const char*
	// fieldName, 	                                                                                              T val)
	// {}

	//	// Encoded values
	//	template<typename T> void write(const char* fieldName, const EncodedInt<T>& val) {}

	//	// Objects
	//	template<typename T>
	//	typename std::enable_if<!is_primitive<T>::value, void>::type write(const char* fieldName, const T& val) {}

	//	// String
	//	void writeString(const char* fieldName, const std::string& val, size_t /* maxSize */) {}

	//	void writeDynString(const char* fieldName, const std::string& val, size_t /* count */) {}

	//	void writeArray(const char* fieldName, const char* val, size_t /* size */) {}

	//	// Fixed array
	//	template<typename T> void writeArray(const char* fieldName, const T* val, size_t size) {}

	//	// Fixed array of primitive with cast
	//	template<typename T, typename U>
	//	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type writeArray(const char* fieldName,
	//	                                                                                   const U* val,
	//	                                                                                   size_t size) {}

	//	// Dynamic array
	//	template<typename T> void writeDynArray(const char* fieldName, const std::vector<T>& val, uint32_t) {}

	//	// Dynamic array of primitive with cast
	//	template<typename T, typename U>
	//	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type writeDynArray(const char* fieldName,
	//	                                                                                      const std::vector<U>& val,
	//	                                                                                      uint32_t count) {}

	//	template<typename T>
	//	typename std::enable_if<is_primitive<T>::value, void>::type writeSize(const char* /* fieldName */, T /* size */)
	//{}

	//	void pad(const char* /* fieldName */, size_t /* size */) {}

	// Read functions /////////////////////////

	void readHeader(uint16_t& id) {}

	// Primitives via arg
	template<typename T, typename U>
	typename std::enable_if<is_primitive<U>::value && !std::is_same<ar_handle_t, U>::value, void>::type read(
	    const char* fieldName, U& val) {}

	// Objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type read(const char* fieldName, T& val) {}

	// String
	void readString(const char* fieldName, std::string& val, size_t size) {}
	void readDynString(const char* fieldName, std::string& val, uint32_t sizeToRead, bool hasNullTerminator) {}
	void readEndString(const char* fieldName, std::string& val, bool hasNullTerminator) {}

	// Fixed array of primitive
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type readArray(const char* fieldName, T* val, size_t size) {}

	// Fixed array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type readArray(const char* fieldName,
	                                                                                  U* val,
	                                                                                  size_t size) {}

	// Fixed array of objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type readArray(const char* fieldName, T* val, size_t size) {

	}

	// Dynamic array of primitive
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type readDynArray(const char* fieldName,
	                                                                         std::vector<T>& val,
	                                                                         uint32_t sizeToRead) {}

	// Dynamic array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type readDynArray(const char* fieldName,
	                                                                                     std::vector<U>& val,
	                                                                                     uint32_t sizeToRead) {}

	// Dynamic array of object
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type readDynArray(const char* fieldName,
	                                                                          std::vector<T>& val,
	                                                                          uint32_t sizeToRead) {}

	// End array with primitive, read to the end of stream
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type readEndArray(const char* fieldName,
	                                                                         std::vector<T>& val) {}

	// End array, read to the end of stream
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type readEndArray(const char* fieldName,
	                                                                          std::vector<T>& val) {}

	// read size for objects (std:: containers)
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type readSize(const char* fieldName, uint32_t& val) {}

	void discard(const char* fieldName, size_t size) {}
};

