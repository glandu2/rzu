#include "TelnetSession.h"
#include "../GlobalConfig.h"
#include <string.h>
#include <stdio.h>
#include "Utils.h"
#include <string>

namespace AdminServer {

TelnetSession::TelnetSession() {
}

void TelnetSession::onDataReceived() {
	std::vector<char> buffer;

	if(getSocket()->getAvailableBytes() > 0) {
		getSocket()->readAll(&buffer);
		parseData(buffer);
	}
}

void TelnetSession::parseData(const std::vector<char>& data) {
	buffer.insert(buffer.end(), data.begin(), data.end());

	char* nextLine;

	while((nextLine = (char*)memchr(buffer.data(), '\n', buffer.size()))) {
		size_t byteCount = nextLine - buffer.data();
		std::string line(buffer.data(), byteCount);
		parseCommand(line);
	}
}

void TelnetSession::parseCommand(const std::string& data) {

}


} //namespace AdminServer
