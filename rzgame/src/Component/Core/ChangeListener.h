#pragma once

#include <stdint.h>

class ChangeListener {
public:
	bool getChangedAndReset() {
		bool hasChanged = this->hasChanged;
		this->hasChanged = false;
		return hasChanged;
	}

	void notifyChanged() { hasChanged = true; }

private:
	bool hasChanged;
};

// clang-format off
#define PROPERTY(type, name) \
    public:\
    type name() { return name##_ ; }\
    void name(type val) { this->name##_ = val; notifyChanged(); } \
    private: type name##_
// clang-format on

template<typename DbMappingClass> class DbQueryJob;
class DbConnectionPool;

template<typename T> class prop {
private:
	template<typename U> friend class DbQueryJob;  // allow database direct access to load data

	T val;
	bool hasChanged_;

public:
	prop() : hasChanged_(false) {}

	prop<T>& operator=(const T& val) {
		if(val != this->val) {
			hasChanged_ = true;
			this->val = val;
		}
		return *this;
	}

	operator const T&() const { return val; }

	const T& operator()() const { return val; }

	bool hasChanged() { return this->hasChanged_; }
	void resetChanged() { this->hasChanged_ = false; }
};

