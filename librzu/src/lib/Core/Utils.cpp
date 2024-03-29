#include "Utils.h"
#include "Config/ConfigParamVal.h"
#include <algorithm>
#include <errno.h>
#include <sstream>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>  //for GetModuleFileName
#else
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#endif

char Utils::applicationPath[260];
char Utils::applicationName[260];
bool Utils::applicationFilePathInitialized;

uint64_t Utils::getTimeInMsec() {
#ifdef _WIN32
	static const uint64_t EPOCH = 116444736000000000 / 10000;
	FILETIME fileTime;

	GetSystemTimeAsFileTime(&fileTime);

	return (uint64_t(fileTime.dwLowDateTime) + (uint64_t(fileTime.dwHighDateTime) << 32)) / 10000L - EPOCH;
#else
	struct timeval tp;
	gettimeofday(&tp, nullptr);
	return (uint64_t) tp.tv_sec * 1000 + (uint64_t) tp.tv_usec / 1000;
#endif
}

struct tm* Utils::getGmTime(time_t secs, struct tm* tm) {
#ifdef _WIN32
	gmtime_s(tm, &secs);
#else
	gmtime_r(&secs, tm);
#endif

	tm->tm_year += 1900;
	tm->tm_mon += 1;
	return tm;
}

time_t Utils::getTimeGm(const struct tm* tm) {
	struct tm tm_copy = *tm;
	tm_copy.tm_year -= 1900;
	tm_copy.tm_mon -= 1;

#ifdef _WIN32
	return _mkgmtime(&tm_copy);
#else
	return timegm(&tm_copy);
#endif
}

int Utils::mkdir(const char* dir) {
#ifdef _WIN32
	if(::mkdir(dir) < 0)
		return errno;
	else
		return 0;
#else
	if(::mkdir(dir, 0755) < 0)
		return errno;
	else
		return 0;
#endif
}

void Utils::getApplicationFilePath() {
	if(applicationFilePathInitialized)
		return;

	char applicationFilePath[260];

	size_t n = sizeof(applicationFilePath);
	if(uv_exepath(applicationFilePath, &n) == 0)
		applicationFilePath[n] = '\0';
	else
		applicationFilePath[0] = '\0';

	if(applicationFilePath[0] == 0)
		strcpy(applicationFilePath, ".");

	// remove file name
	size_t len = strlen(applicationFilePath);
	char* p = applicationFilePath + len;
	while(p >= applicationFilePath) {
		if(*p == '/' || *p == '\\')
			break;
		p--;
	}
	if(p >= applicationFilePath)
		*p = 0;
	strcpy(applicationPath, applicationFilePath);
	if(p < applicationFilePath + len)
		strcpy(applicationName, p + 1);
	else
		applicationName[0] = '\0';

	for(p = applicationName; *p != 0; p++) {
		if(*p == '.') {
			*p = '\0';
			break;
		}
	}

	applicationFilePathInitialized = true;
}

const char* Utils::getApplicationPath() {
	getApplicationFilePath();

	return applicationPath;
}

const char* Utils::getApplicationName() {
	getApplicationFilePath();

	return applicationName;
}

std::string Utils::getFullPath(const std::string& partialPath) {
	if(partialPath.size() >= 2 && partialPath.at(0) == '.' && (partialPath.at(1) == '/' || partialPath.at(1) == '\\'))
		return partialPath;

	if(isAbsolute(partialPath.c_str()))
		return partialPath;

	return std::string(Utils::getApplicationPath()) + '/' + partialPath;
}

bool Utils::isAbsolute(const char* dir) {
#ifdef _WIN32
	if(isalpha(dir[0]) && dir[1] == ':' && (dir[2] == '\\' || dir[2] == '/'))
		return true;
	else
		return false;
#else
	if(dir[0] == '/')
		return true;
	else
		return false;
#endif
}

void Utils::autoSetAbsoluteDir(cval<std::string>& value) {
	value.addListener(nullptr, &autoSetAbsoluteDirConfigValue);
	autoSetAbsoluteDirConfigValue(nullptr, &value);
}

std::string Utils::convertToString(int i) {
	char buffer[16];
	snprintf(buffer, sizeof(buffer), "%d", i);
	return std::string(buffer);
}

std::string Utils::convertToString(float i) {
	char buffer[128];
	snprintf(buffer, sizeof(buffer), "%f", i);
	return std::string(buffer);
}

std::string Utils::convertToString(const char* str, int maxSize) {
	return std::string(str, std::find(str, str + std::max(0, maxSize), '\0'));
}

std::vector<unsigned char> Utils::convertToDataArray(const unsigned char* data, int maxSize, int usedSize) {
	return std::vector<unsigned char>(data, data + std::max(0, std::min(maxSize, usedSize)));
}

std::vector<unsigned char> Utils::convertToDataArray(const unsigned char* data, int size) {
	return std::vector<unsigned char>(data, data + std::max(0, size));
}

void Utils::convertDataToHex(const void* data, size_t size, char* outHex) {
	static const char hexMapping[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	size_t i;
	for(i = 0; i < size; i++) {
		outHex[i * 2] = hexMapping[(static_cast<const char*>(data)[i] >> 4) & 0x0F];
		outHex[i * 2 + 1] = hexMapping[static_cast<const char*>(data)[i] & 0x0F];
	}
	outHex[i * 2] = '\0';
}

std::vector<unsigned char> Utils::convertHexToData(const std::string& hex) {
	size_t i;
	size_t size = hex.size() / 2;
	std::vector<unsigned char> result;

	result.reserve(size);

	for(i = 0; i < size; i++) {
		unsigned char c = hex[i * 2];
		unsigned char val = 0;

		if(c >= '0' && c <= '9')
			val = (c - '0') << 4;
		else if(c >= 'A' && c <= 'F')
			val = (c - 'A' + 10) << 4;
		else if(c >= 'a' && c <= 'f')
			val = (c - 'a' + 10) << 4;

		c = hex[i * 2 + 1];

		if(c >= '0' && c <= '9')
			val |= (c - '0');
		else if(c >= 'A' && c <= 'F')
			val |= c - 'A' + 10;
		else if(c >= 'a' && c <= 'f')
			val |= c - 'a' + 10;

		result.push_back(val);
	}

	return result;
}

void Utils::autoSetAbsoluteDirConfigValue(IListener*, cval<std::string>* value) {
	std::string dir = value->get();
	std::string fullPath;

	if(isAbsolute(dir.c_str()))
		return;

	fullPath = getFullPath(dir);

	// keep defaultness
	if(value->setDefault(fullPath, false) == false)
		value->set(fullPath, false);
}

void* Utils::memmem(const void* haystack, size_t hlen, const void* needle, size_t nlen) {
	int needle_first;
	const char* p = (const char*) haystack;
	size_t plen = hlen;

	if(!nlen)
		return NULL;

	needle_first = *(unsigned char*) needle;

	while(plen >= nlen && (p = (const char*) memchr(p, needle_first, plen - nlen + 1))) {
		if(!memcmp(p, needle, nlen))
			return (void*) p;

		p++;
		plen = hlen - (p - (const char*) haystack);
	}

	return NULL;
}

unsigned long Utils::getPid() {
#ifdef _WIN32
	return GetCurrentProcessId();
#else
	return getpid();
#endif
}

static int c99vsnprintf(char* dest, size_t size, const char* format, va_list args) {
	va_list argsForCount;
	va_copy(argsForCount, args);

	int result = vsnprintf(dest, size, format, args);

#ifdef _WIN32
	if(result == -1)
		result = _vscprintf(format, argsForCount);
#endif

	va_end(argsForCount);

	return result;
}

void Utils::stringFormatv(std::string& dest, const char* message, va_list args) {
	va_list argsFor2ndPass;
	va_copy(argsFor2ndPass, args);

	dest.resize(128);
	int result = c99vsnprintf(&dest[0], dest.size(), message, args);

	if(result < 0) {
		dest = message;
		va_end(argsFor2ndPass);
		return;
	}

	if(result < (int) dest.size()) {
		dest.resize(result);
	} else if(result >= (int) dest.size()) {
		dest.resize(result + 1);

		vsnprintf(&dest[0], dest.size(), message, argsFor2ndPass);
		dest.resize(result);
	}
	va_end(argsFor2ndPass);
}

bool Utils::stringReplace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if(start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

void Utils::stringReplaceAll(std::string& str, const std::string& from, const std::string& to) {
	if(from.empty())
		return;
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();  // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

bool Utils::stringWildcardMatch(const char* pTameText, const char* pWildText) {
	// http://www.drdobbs.com/architecture-and-design/matching-wildcards-an-empirical-way-to-t/240169123
	// These two values are set when we observe a wildcard character.  They
	// represent the locations, in the two strings, from which we start once
	// we've observed it.
	//
	const char* pTameBookmark = nullptr;
	const char* pWildBookmark = nullptr;

	// Walk the text strings one character at a time.
	while(1) {
		// How do you match a unique text string?
		if(*pWildText == '*') {
			// Easy: unique up on it!
			while(*(++pWildText) == '*') {
			}  // "xy" matches "x**y"

			if(!*pWildText)
				return true;  // "x" matches "*"

			if(*pWildText != '?') {
				// Fast-forward to next possible match.
				while(*pTameText != *pWildText) {
					if(!(*(++pTameText)))
						return false;  // "x" doesn't match "*y*"
				}
			}

			pWildBookmark = pWildText;
			pTameBookmark = pTameText;
		} else if(*pTameText != *pWildText && *pWildText != '?') {
			// Got a non-match.  If we've set our bookmarks, back up to one
			// or both of them and retry.
			//
			if(pWildBookmark) {
				if(pWildText != pWildBookmark) {
					pWildText = pWildBookmark;

					if(*pTameText != *pWildText) {
						// Don't go this far back again.
						pTameText = ++pTameBookmark;
						continue;  // "xy" matches "*y"
					} else {
						pWildText++;
					}
				}

				if(*pTameText) {
					pTameText++;
					continue;  // "mississippi" matches "*sip*"
				}
			}

			return false;  // "xy" doesn't match "x"
		}

		pTameText++;
		pWildText++;

		// How do you match a tame text string?
		if(!*pTameText) {
			// The tame way: unique up on it!
			while(*pWildText == '*')
				pWildText++;  // "x" matches "x*"

			if(!*pWildText)
				return true;  // "x" matches "x"

			return false;  // "x" doesn't match "xy"
		}
	}
}

std::vector<std::string> Utils::parseCommand(const std::string& data) {
	std::vector<std::string> args;
	std::ostringstream arg;

	const char* p;
	bool insideQuotes = false;

	for(p = data.c_str(); p < data.c_str() + data.size(); p++) {
		if(*p == '\"') {
			if(p + 1 < data.c_str() + data.size() && *(p + 1) == '\"') {
				p++;
				arg << '\"';
			} else {
				insideQuotes = !insideQuotes;
			}
		} else if(insideQuotes == false && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
			if(arg.tellp()) {
				args.push_back(arg.str());
				arg.str("");
				arg.clear();
			}
		} else {
			arg << *p;
		}
	}
	if(arg.tellp())
		args.push_back(arg.str());

	return args;
}

std::vector<std::string> Utils::stringSplit(const std::string& data, char delim) {
	std::vector<std::string> result;
	size_t startOffset = 0;
	size_t endOffset;
	while((endOffset = data.find_first_of(delim, startOffset)) != std::string::npos) {
		result.push_back(data.substr(startOffset, endOffset - startOffset));
		startOffset = endOffset + 1;
	}
	if(startOffset < data.size())
		result.push_back(data.substr(startOffset, std::string::npos));

	return result;
}

void Utils::stringFormat(std::string& dest, const char* message, ...) {
	va_list args;
	va_start(args, message);
	stringFormatv(dest, message, args);
	va_end(args);
}
