#pragma once

#include "Core/Object.h"
#include <string>

class RZU_EXTERN ConfigValue : public Object {
	DECLARE_CLASS(ConfigValue)
public:
	enum Type { Bool, Integer, Float, String };

	ConfigValue(Type type);
	virtual ~ConfigValue() {}

	Type getType() { return type; }
	char getTypeLetter();
	void setKeyName(const std::string* keyName) { this->keyName = keyName; }

	virtual bool getBool();
	virtual int getInt();
	virtual float getFloat();
	virtual std::string getString();

	virtual bool getBool(bool def);
	virtual int getInt(int def);
	virtual float getFloat(float def);
	virtual std::string getString(const char* def);
	virtual std::string getString(const std::string& def);

	virtual void setBool(bool val);
	virtual void setInt(int val);
	virtual void setFloat(float val);
	virtual void setString(const char* val);
	virtual void setString(const std::string& val);

	virtual bool isDefault();

	void setHidden(bool enable) { hidden = enable; }
	bool isHidden() { return hidden; }

	void print();

protected:
	void wrongType(Type expectedType);

private:
	const std::string* keyName;
	Type type;
	bool hidden;
};

template<typename T> class ConfigTypedValue : public ConfigValue {};

template<> class ConfigTypedValue<bool> : public ConfigValue {
public:
	ConfigTypedValue() : ConfigValue(Bool) {}
	virtual bool get() = 0;
	virtual bool getBool() { return get(); }

	virtual bool get(const bool& def) = 0;
	virtual bool getBool(bool def) { return get(def); }

	virtual void set(const bool& val, bool dispatch = true) = 0;
	virtual void setBool(bool val) { return set(val); }
};

template<> class ConfigTypedValue<int> : public ConfigValue {
public:
	ConfigTypedValue() : ConfigValue(Integer) {}
	virtual int get() = 0;
	virtual int getInt() { return get(); }

	virtual int get(const int& def) = 0;
	virtual int getInt(int def) { return get(def); }

	virtual void set(const int& val, bool dispatch = true) = 0;
	virtual void setInt(int val) { return set(val); }
};

template<> class ConfigTypedValue<float> : public ConfigValue {
public:
	ConfigTypedValue() : ConfigValue(Float) {}
	virtual float get() = 0;
	virtual float getFloat() { return get(); }

	virtual float get(const float& def) = 0;
	virtual float getFloat(float def) { return get(def); }

	virtual void set(const float& val, bool dispatch = true) = 0;
	virtual void setFloat(float val) { return set(val); }
};

template<> class ConfigTypedValue<std::string> : public ConfigValue {
public:
	ConfigTypedValue() : ConfigValue(String) {}
	virtual std::string get() = 0;
	virtual std::string getString() { return get(); }

	virtual std::string get(const std::string& def) = 0;
	virtual std::string getString(const char* def) { return get(std::string(def)); }
	virtual std::string getString(const std::string& def) { return get(def); }

	virtual void set(const std::string& val, bool dispatch = true) = 0;
	virtual void setString(const char* val) { return set(std::string(val)); }
	virtual void setString(const std::string& val) { return set(val); }
};

