#include "TelnetSession.h"
#include "../GlobalConfig.h"
#include <string.h>
#include <stdio.h>
#include "Utils.h"
#include <string>
#include "CommandRunner.h"
#include <sstream>

namespace AdminServer {

static const char MSG_WELCOME[] = "Auth server - Administration server - Type \"help\" for a list of available commands\r\n> ";

static const char MSG_UNKNOWN_COMMAND[] = "Unknown command\r\n";
static const char MSG_HELP[] =
		"Available commands:\r\n"
		"- start <server_name>     Start the server <server_name>. Servers names are listed when starting. If <server_name> is \"all\", all server with autostart on are started.\r\n"
		"- stop <server_name>      Same as start <server_name> but stop the server. \"stop all\" will stop all servers and exit.\r\n"
		"- set <variable> <value>  Set config variable <variable> to <value>. Double-quotes are allowed for values with spaces. Use \"\" for a escaped double quote character.\r\n"
		"- get <variable>          Get config variable value.\r\n"
		"- list                    List all connected gameservers and information about them.\r\n"
		"- mem                     List object counts.\r\n"
		"- closedb                 Close all idle database connections (use this to bring a database offline).\r\n"
		"\r\n";

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
	else if(args.size() > 0 && args[0] == "closedb")
		commandRunner->closeDbConnections();
	else if(args.size() > 0 && args[0] == "help")
		write(MSG_HELP, sizeof(MSG_HELP));
	else
		write(MSG_UNKNOWN_COMMAND, sizeof(MSG_UNKNOWN_COMMAND));
}


} //namespace AdminServer
