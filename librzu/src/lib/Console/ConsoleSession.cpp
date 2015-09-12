#include "ConsoleSession.h"
#include "NetSession/ServersManager.h"
#include "Config/ConfigInfo.h"
#include <stdlib.h>
#include "ClassCounter.h"
#include "Database/DbConnectionPool.h"
#include "Core/CrashHandler.h"
#include <string.h>
#include "Core/Utils.h"
#include "Config/GlobalCoreConfig.h"
#include "ConsoleCommands.h"
#include "../NetSession/SessionServer.h"
#include "Config/GlobalCoreConfig.h"

void ConsoleSession::start(ServersManager* serverManager) {
	static SessionServer<ConsoleSession> adminConsoleServer(
				GlobalCoreConfig::get()->admin.listener.listenIp,
				GlobalCoreConfig::get()->admin.listener.port,
				&GlobalCoreConfig::get()->admin.listener.idleTimeout);
	serverManager->addServer("admin.console", &adminConsoleServer, GlobalCoreConfig::get()->admin.listener.autoStart, true);
}

ConsoleSession::ConsoleSession() : consoleCommands(ConsoleCommands::get()) {
	Object::log(LL_Info, "New console session\n");
}

ConsoleSession::~ConsoleSession() {
	Object::log(LL_Info, "Console session closed\n");
}

void ConsoleSession::write(const char *message) {
	TelnetSession::write(message, strlen(message));
}

void ConsoleSession::writef(const char *format, ...) {
	std::string str;
	va_list args;

	va_start(args, format);
	Utils::stringFormatv(str, format, args);
	va_end(args);

	TelnetSession::write(str.c_str(), str.size());
}

void ConsoleSession::log(const char *message, ...) {
	va_list args;
	va_start(args, message);
	Object::logv(LL_Info, message, args);
	va_end(args);
}

void ConsoleSession::onConnected() {
	writef("%s - Administration console - Type \"help\" for a list of available commands\r\n",
		   GlobalCoreConfig::get()->app.appName.get().c_str());
	printPrompt();
}

void ConsoleSession::printPrompt() {
	if(getStream() && getStream()->getState() == Stream::ConnectedState)
		write("> ");
}

void ConsoleSession::onCommand(const std::vector<std::string> &args) {
	if(args.empty()) {
		printPrompt();
		return;
	}

	const std::string& commandName = args[0];
	std::vector<std::string> commandArgs(args.begin()+1, args.end());

	const ConsoleCommands::Command* command = consoleCommands->getCommand(commandName);

	if(!command) {
		writef("Unknown command : %s\r\n", commandName.c_str());
	} else {
		ConsoleCommands::Command::CallStatus status = command->call(this, commandArgs);
		switch(status) {
			case ConsoleCommands::Command::CS_Success:
				break;

			case ConsoleCommands::Command::CS_NotEnoughArgs:
				write("Not enough arguments\r\n");
				break;

			case ConsoleCommands::Command::CS_TooMuchArgs:
				write("Too many arguments\r\n");
				break;
		};

		if(status != ConsoleCommands::Command::CS_Success) {
			writef("Usage:\r\n%s\r\n", command->usageExample.c_str());
		}
	}

	printPrompt();
}
