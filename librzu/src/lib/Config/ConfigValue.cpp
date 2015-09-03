#include "ConfigValue.h"
#include "Core/Utils.h"
#include <stdlib.h>

ConfigValue::ConfigValue(Type type)
	: keyName(nullptr), type(type), hidden(false)
{
}


bool ConfigValue::getBool() {
	return getBool(false);
}

int ConfigValue::getInt() {
	return getInt(0);
}

float ConfigValue::getFloat() {
	return getFloat(0.0);
}

std::string ConfigValue::getString() {
	switch(getType()) {
		case ConfigValue::Bool:
			return getBool() ? "true" : "false";

		case ConfigValue::Integer:
			return Utils::convertToString(getInt());

		case ConfigValue::Float:
			return Utils::convertToString(getFloat());

		case ConfigValue::String:
			return getString(nullptr);
	}

	return getString(nullptr);
}


bool ConfigValue::getBool(bool def) {
	wrongType(Bool);
	return def;
}

int ConfigValue::getInt(int def) {
	wrongType(Integer);
	return def;
}

float ConfigValue::getFloat(float def) {
	wrongType(Float);
	return def;
}

std::string ConfigValue::getString(const char* def) {
	wrongType(String);
	if(def)
		return std::string(def);
	else
		return std::string();
}

std::string ConfigValue::getString(const std::string& def) {
	wrongType(String);
	return def;
}


void ConfigValue::setBool(bool def) {
	wrongType(Bool);
}

void ConfigValue::setInt(int def) {
	wrongType(Integer);
}

void ConfigValue::setFloat(float def) {
	wrongType(Float);
}

void ConfigValue::setString(const char* def) {
	wrongType(String);
}

void ConfigValue::setString(const std::string& def) {
	wrongType(String);
}

bool ConfigValue::isDefault() {
	return false;
}

void ConfigValue::wrongType(Type expectedType) {
	if(expectedType != type) {
		static const char* typeStr[] = {
			"Bool",
			"Integer",
			"Float",
			"String"
		};

		fatal("Config value type mismatch for %s[%s], tried to get as %s. Check the config file.\n",
				keyName != nullptr ? keyName->c_str() : "unknown key",
				typeStr[type],
				typeStr[expectedType]);
		abort();
	}
}