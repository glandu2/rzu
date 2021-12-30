#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vector>

template<typename T> class HandleGenerator {
public:
	HandleGenerator(size_t reserveSize = 256);

	T allocateId();
	void freeId(T id);

private:
	std::vector<T> freeIds;
	T greatestUsedId = 0;
};

template<typename T> HandleGenerator<T>::HandleGenerator(size_t reserveSize) {
	freeIds.reserve(reserveSize);
}
template<typename T> T HandleGenerator<T>::allocateId() {
	if(freeIds.empty())
		return greatestUsedId++;

	T value = freeIds.front();
	freeIds.pop_back();
	return value;
}
template<typename T> void HandleGenerator<T>::freeId(T id) {
	freeIds.emplace_back(id);
}
