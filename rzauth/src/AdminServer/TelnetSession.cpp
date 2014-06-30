#include "TelnetSession.h"
#include "../GlobalConfig.h"
#include <string.h>
#include <stdio.h>
#include "Utils.h"
#include <string>
#include "CommandRunner.h"
#include <sstream>

namespace AdminServer {

static const char MSG_WELCOME[] = "Auth server - Administration server\r\n> ";
static const char MSG_UNKNOWN_COMMAND[] = "Unknown command\r\n";

TelnetSession::TelnetSession() {
	commandRunner = new CommandRunner(this);
	info("New admin session\n");
}

TelnetSession::~TelnetSession() {
	info("Admin session closed\n");
	delete commandRunner;
}

void TelnetSession::onDataReceived() {
	std::vector<char> buffer;

	if(getSocket()->getAvailableBytes() > 0) {
		getSocket()->readAll(&buffer);
		parseData(buffer);
	}
}

void TelnetSession::onConnected() {
	write(MSG_WELCOME, sizeof(MSG_WELCOME));
}

void TelnetSession::parseData(const std::vector<char>& data) {
	buffer.insert(buffer.end(), data.begin(), data.end());

	char* nextLine;

	while((nextLine = (char*)memchr(buffer.data(), '\n', buffer.size()))) {
		size_t byteCount = nextLine - buffer.data() + 1; //+1, count the found \n
		std::string line(buffer.data(), byteCount);
		parseCommand(line);
		buffer.erase(buffer.begin(), buffer.begin() + byteCount);
		write("> ", 2);
	}
}

void TelnetSession::parseCommand(const std::string& data) {
	std::vector<std::string> args;
	std::ostringstream arg;

	const char *p;
	p = data.c_str();
	bool insideQuotes = false;

	for(p = data.c_str(); p < data.c_str() + data.size(); p++) {
		if(*p == '\"') {
			if(p+1 < data.c_str() + data.size() && *(p+1) == '\"') {
				p++;
				arg << '\"';
			} else {
				insideQuotes = !insideQuotes;
			}
		} else if(insideQuotes == false && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
			args.push_back(arg.str());
			arg.str("");
			arg.clear();
		} else {
			arg << *p;
		}
	}
	if(arg.tellp())
		args.push_back(arg.str());

	if(args.size() > 1 && args[0] == "start")
		commandRunner->startServer(args[1]);
	else if(args.size() > 1 && args[0] == "stop")
		commandRunner->stopServer(args[1]);
	else if(args.size() > 2 && args[0] == "set")
		commandRunner->setEnv(args[1], args[2]);
	else if(args.size() > 1 && args[0] == "get")
		commandRunner->getEnv(args[1]);
	else if(args.size() > 0 && args[0] == "list")
		commandRunner->listGameServers();
	else if(args.size() > 0 && args[0] == "mem")
		commandRunner->listObjectsCount();
	else
		write(MSG_UNKNOWN_COMMAND, sizeof(MSG_UNKNOWN_COMMAND));
}


} //namespace AdminServer
