#include "ConfigInfo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ConfigValue::ConfigValue(Type type) : type(type), keyName(nullptr) {

}

bool ConfigValue::check(Type expectedType, bool soft) {
	if(expectedType != type && type != None) {
		const char* typeStr[] = {
			"Bool",
			"Integer",
			"Float",
			"String",
			"None"
		};
		fprintf(stderr, "%s: config value type mismatch for %s[%s], tried to get as %s. Check the config file.\n",
				soft ? "Warning" : "Fatal",
				keyName != nullptr ? keyName->c_str() : "unknown key",
				typeStr[type],
				typeStr[expectedType]);
		if(!soft)
			abort();
		return false;
	}
	return true;
}

ConfigInfo::ConfigInfo() {

}

ConfigValue* ConfigInfo::get(const std::string& key) {
	std::unordered_map<std::string, ConfigValue*>::const_iterator it;

	it = config.find(key);
	if(it != config.cend()) {
		return it->second;
	} else {
		ConfigValue* v = new ConfigValue(ConfigValue::None);
		addValue(key, v);
		return v;
	}
}

std::pair<std::unordered_map<std::string, ConfigValue*>::iterator, bool> ConfigInfo::addValue(const std::string& key, ConfigValue* v) {
	std::pair<std::unordered_map<std::string, ConfigValue*>::iterator, bool> it = config.emplace(key, v);
	if(it.second)
		v->setKeyName(&it.first->first);
	return it;
}

bool ConfigInfo::readFile(const char* filename) {
	FILE* file;
	char line[1024];
	char* p;
	ConfigValue* v;
	typedef std::unordered_map<std::string, ConfigValue*>::iterator Iterator;

	file = fopen(filename, "rb");
	if(!file)
		return false;

	while(fgets(line, 1024, file)) {
		int len = strlen(line);
		if(len < 3)   //minimum: type + space + key char
			continue;

		p = line + len - 1;
		while(isspace(*p))
			p--;
		*(p+1) = 0;

		p = strchr(line, ':');
		if(!p)
			continue;
		*p = 0;
		p++;

		switch(line[0]) {
			case 'B':
				v = new ConfigValue(ConfigValue::Bool);
				v->set(!strcmp(p, "true"));
				break;

			case 'N':
			case 'I':
				v = new ConfigValue(ConfigValue::Integer);
				v->set(atoi(p));
				break;

			case 'F':
				v = new ConfigValue(ConfigValue::Float);
				v->set(atof(p));
				break;

			case 'S':
				v = new ConfigValue(ConfigValue::String);
				v->set(std::string(p));
				break;

			default:
				continue;
		}
		std::pair<Iterator, bool> it = addValue(std::string(line+2), v);
		if(it.second == false) {
			ConfigValue* orig = it.first->second;
			orig->set(v);
		}

		v = nullptr;
	}

	fclose(file);

	return true;
}

bool ConfigInfo::writeFile(const char *filename) {
	FILE* file;

	file = fopen(filename, "wb");
	if(!file)
		return false;
	dump(file);
	fclose(file);

	return true;
}

void ConfigInfo::dump(FILE *out) {
	std::unordered_map<std::string, ConfigValue*>::const_iterator it, itEnd;
	std::string val;

	for(it = config.cbegin(), itEnd = config.cend(); it != itEnd; ++it) {
		ConfigValue* v = it->second;
		char type = 'd';
		switch(v->getType()) {
			case ConfigValue::Bool:
				type = 'B';
				val = v->get(false) ? "true" : "false";
				break;

			case ConfigValue::Integer:
				type = 'N';
				val = std::to_string(v->get(0));
				break;

			case ConfigValue::Float:
				type = 'F';
				val = std::to_string(v->get(0.0f));
				break;

			case ConfigValue::String:
				type = 'S';
				val = v->get("<NULL>");
				break;

			case ConfigValue::None:
				type = '0';
				val = "<NONE>";
				break;
		}

		fprintf(out, "%c %s:%s\n", type, it->first.c_str(), val.c_str());
	}
}
