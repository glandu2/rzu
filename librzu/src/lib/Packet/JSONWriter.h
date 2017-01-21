#ifndef PACKETS_JSONWRITER_H
#define PACKETS_JSONWRITER_H

#include "StructSerializer.h"
#include <sstream>
#include "EncodedInt.h"

class JSONWriter : public StructSerializer {
private:
	std::stringstream json;
	int depth;
	bool newList;
	const bool compact;

private:
	const char* getEndFieldSeparator() { return compact ? "\":" : "\": "; }

public:
	JSONWriter(int version, bool compact) : StructSerializer(version), depth(1), newList(true), compact(compact) {
		json << "{";
	}

	void finalize() { json << (compact ? "}\n" : "\n}\n"); newList = true; }
	void start() { json << "{"; }
	void clear() { json.str(std::string()); json.clear(); }

	std::string toString() { return json.str(); }

	void printIdent(bool addNewLine = true) {
		if(!newList)
			json << (compact ? "," : ", ");

		if(addNewLine && !compact) {
			json << "\n";
			for(int i = 0; i < depth; i++)
				json << '\t';
		}

		newList = false;
	}

	// Write functions /////////////////////////

	//handle ambiguous << operator for int16_t and int8_t
	template<typename T>
	typename std::enable_if<is_primitive<T>::value && sizeof(T) < sizeof(int), void>::type
	write(const char* fieldName, T val) {
		printIdent(fieldName != nullptr);
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << (int)val;
	}

	//Primitives
	template<typename T>
	typename std::enable_if<is_primitive<T>::value && sizeof(T) >= sizeof(int), void>::type
	write(const char* fieldName, T val) {
		printIdent(fieldName != nullptr);
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << val;
	}

	//Encoded values
	template<typename T>
	void write(const char* fieldName, const EncodedInt<T>& val) {
		printIdent(fieldName != nullptr);
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << (uint32_t)val;
	}

	//Objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type
	write(const char* fieldName, const T& val) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << '{';

		newList = true;
		depth++;
		val.serialize(this);
		depth--;

		newList = true;
		printIdent();
		json << '}';
	}

	//String
	void writeString(const char* fieldName, const std::string& val, size_t maxSize) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << "\"" << val << "\"";
	}

	void writeDynString(const char* fieldName, const std::string& val, bool hasNullTerminator) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << "\"" << val << "\"";
	}

	void writeArray(const char* fieldName, const char* val, size_t size) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << "\"" << val << "\"";
	}

	//Fixed array
	template<typename T>
	void writeArray(const char* fieldName, const T* val, size_t size) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << '[';

		newList = true;
		depth++;
		for(size_t i = 0; i < size; i++) {
			write(nullptr, val[i]);
		}
		depth--;

		newList = true;
		printIdent(false);
		json << ']';
	}

	//Fixed array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type
	writeArray(const char* fieldName, const U* val, size_t size) {
		writeArray<U>(fieldName, val, size);
	}

	//Dynamic array
	template<typename T>
	void writeDynArray(const char* fieldName, const std::vector<T>& val) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << '[';

		newList = true;
		depth++;
		auto it = val.begin();
		auto itEnd = val.end();
		for(; it != itEnd; ++it)
			write(nullptr, *it);
		depth--;

		newList = true;
		printIdent(false);
		json << ']';
	}

	//Dynamic array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type
	writeDynArray(const char* fieldName, const std::vector<U>& val) {
		writeDynArray<U>(fieldName, val);
	}

	void pad(const char* fieldName, size_t size) {
	}

	// Read functions /////////////////////////

	//Primitives via arg
	template<typename T, typename U>
	typename std::enable_if<is_primitive<U>::value, void>::type
	read(const char* fieldName, U& val) {
	}

	//Objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type
	read(const char* fieldName, T& val) {
	}

	//String
	void readString(const char* fieldName, std::string& val, size_t size) {}
	void readDynString(const char* fieldName, std::string& val, bool hasNullTerminator) {}

	//Fixed array of primitive
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type
	readArray(const char* fieldName, T* val, size_t size) {
	}

	//Fixed array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type
	readArray(const char* fieldName, U* val, size_t size) {
	}

	//Fixed array of objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type
	readArray(const char* fieldName, T* val, size_t size) {
	}

	//Dynamic array of primitive
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type
	readDynArray(const char* fieldName, std::vector<T>& val) {
	}

	//Dynamic array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type
	readDynArray(const char* fieldName, std::vector<U>& val) {
	}

	//Dynamic array of object
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type
	readDynArray(const char* fieldName, std::vector<T>& val) {
	}

	//read size for objects (std:: containers)
	template<typename T, class U>
	void readSize(const char* fieldName, U& vec) {
	}

	void discard(const char* fieldName, size_t size) {
	}
};

#endif /* PACKETS_JSONWRITER_H */
