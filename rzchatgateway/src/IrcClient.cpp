#include "IrcClient.h"
#include "Utils.h"
#include <algorithm>
#include "GameSession.h"
#include "Packets/TS_CS_CHAT_REQUEST.h"

IrcClient::IrcClient(const std::string& ip, int port, const std::string& hostname, const std::string& channel, const std::string& nickname, Log* packetLog)
	: gameSession(nullptr), channelName(channel), nickname(nickname), ip(ip), hostname(hostname), port(port), joined(false)
{
}

void IrcClient::connect(std::string servername) {
	this->servername = servername.substr(0, servername.find_first_of('('));
	SocketSession::connect(ip.c_str(), port);
}

void IrcClient::onStateChanged(Stream::State oldState, Stream::State newState) {
	if(newState == Stream::ConnectedState) {
		char loginText[128];
		std::string lowerCaseServerName;

		if(nickname.size() == 0)
			nickname = servername;

		lowerCaseServerName = servername;
		lowerCaseServerName[0] = tolower(lowerCaseServerName[0]);

		if(channelName.size() == 0)
			channelName = "#rappelz-" + lowerCaseServerName;

		sprintf(loginText, "NICK %s\r\nUSER gw-%s gw-%s %s :rzchatgateway %s\r\n",
				nickname.c_str(),
				lowerCaseServerName.c_str(),
				lowerCaseServerName.c_str(),
				hostname.c_str(),
				servername.c_str());

		write(loginText, strlen(loginText));
	} else if(newState == Stream::UnconnectedState) {
		joined = false;
		connect(servername);
	}
}

void IrcClient::onDataReceived() {
	std::vector<char> dataRecv;
	getStream()->readAll(&dataRecv);
	char *p;

	buffer.insert(buffer.end(), dataRecv.begin(), dataRecv.end());
	while(buffer.size() > 0 && (p = (char*)Utils::memmem(&buffer[0], buffer.size(), "\r\n", 2))) {
		std::string line(&buffer[0], p);
		buffer.erase(buffer.begin(), buffer.begin() + (p - &buffer[0]) + 2);
		onIrcLine(line);
	}
}

void IrcClient::onIrcLine(const std::string& line) {
	std::string prefix;
	std::string command;
	std::string parameters;
	std::string trailing;

	trace("IRC line received: %s\n", line.c_str());

	parseIrcMessage(line, prefix, command, parameters, trailing);

	if(command == "001") {
		char login[128];
		sprintf(login, "JOIN %s\r\n", channelName.c_str());
		write(login, strlen(login));
		joined = true;
		info("Joined IRC channel %s at %s:%d as user %s\n", channelName.c_str(), ip.c_str(), port, nickname.c_str());
	} else if(command == "PING") {
		std::string pong = "PONG :" + trailing;
		write(pong.c_str(), pong.size());
	} else if(command == "PRIVMSG") {
		std::string sender;

		if(trailing.size() == 0)
			return;

		//ignore special commands
		if(trailing[0] < ' ')
			return;

		sender.assign(prefix, 0, prefix.find_first_of('!'));

		switch(trailing[0]) {
			case '\"': {
				std::string target, msg;
				size_t separator = trailing.find_first_of(' ');

				target.assign(trailing, 1, separator-1);

				if(separator == std::string::npos || separator == trailing.size() - 1) {
					sendMessage(sender.c_str(), "Utilisation des messages priv\xE9s comme en jeu: \"Player message \xE0 envoyer");
					break;
				}

				msg.assign(trailing, separator+1, std::string::npos);
				mpRepliesPlayers[target] = sender;
				gameSession->sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_WHISPER, sender.c_str(), target.c_str(), msg.c_str());
				break;
			}

			case '$':
				gameSession->sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_ADV, sender.c_str(), "", trailing.c_str()+1);
				break;

			case '!':
				gameSession->sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_GLOBAL, sender.c_str(), "", trailing.c_str()+1);
				break;

			case '#':
				gameSession->sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_PARTY, sender.c_str(), "", trailing.c_str()+1);
				break;

			case '%':
				gameSession->sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_GUILD, sender.c_str(), "", trailing.c_str()+1);
				break;

			default:
				gameSession->sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_NORMAL, sender.c_str(), "", trailing.c_str());
		}
	}
}

void IrcClient::sendMessage(const char* target, const std::string& msg) {
	char msgText[512];
	if(target && (target[0] == '\0' || target[0] == '#'))
		sprintf(msgText, "PRIVMSG %s :%s\r\n", channelName.c_str(), msg.c_str());
	else
		sprintf(msgText, "PRIVMSG %s :%s\r\n", target, msg.c_str());
	write(msgText, strlen(msgText));
}


const char* IrcClient::getChatColor(int type) {
	switch(type) {
		case TS_CS_CHAT_REQUEST::CHAT_ADV: return "02";
		case TS_CS_CHAT_REQUEST::CHAT_WHISPER: return "08";
		case TS_CS_CHAT_REQUEST::CHAT_GLOBAL: return "15";
		case TS_CS_CHAT_REQUEST::CHAT_EMOTION: return "15";
		case TS_CS_CHAT_REQUEST::CHAT_GM: return "07";
		case TS_CS_CHAT_REQUEST::CHAT_GM_WHISPER: return "08";
		case TS_CS_CHAT_REQUEST::CHAT_PARTY: return "03";
		case TS_CS_CHAT_REQUEST::CHAT_GUILD: return "06";
		case TS_CS_CHAT_REQUEST::CHAT_ATTACKTEAM: return "06";
		case TS_CS_CHAT_REQUEST::CHAT_NOTICE: return "15";
		case TS_CS_CHAT_REQUEST::CHAT_ANNOUNCE: return "15";
		case TS_CS_CHAT_REQUEST::CHAT_CENTER_NOTICE: return "15";
	}
	return nullptr;
}

void IrcClient::sendMsgToIRC(int type, const char* sender, std::string msg) {
	char messageFull[500];

	std::replace(msg.begin(), msg.end(), '\x0D', ' ');
	std::replace(msg.begin(), msg.end(), '\x0A', ' ');

	const char* color = getChatColor(type);

	debug("[GS] Msg %d: %s: %s\n", type, sender, msg.c_str());

	if(sender[0] == '@')
		return;

	if(msg.size() < 1)
		return;

	if(type == TS_CS_CHAT_REQUEST::CHAT_WHISPER) {
		size_t separator = msg.find(": ");
		std::string target;

		if(separator == std::string::npos || separator == msg.size()-2) {
			auto it = mpRepliesPlayers.find(std::string(sender));
			separator = 0;

			if(it == mpRepliesPlayers.end()) {
//					gameSession->sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_WHISPER, "", sender, "Pour envoyer un message priv\xE9, il faut indiquer le nom suivi de \": \". Exemple:");
//					gameSession->sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_WHISPER, "", sender, "Player: message \xE0 envoyer");
			} else {
				target = it->second;
			}
		} else {
			target.assign(msg, 0, separator);
			separator += 2;
		}

		if(color)
			sprintf(messageFull, "%c%s%s: %s%c", 0x03, color, sender, msg.substr(separator).c_str(), 0x03);
		else
			sprintf(messageFull, "%s: %s", sender, msg.substr(separator).c_str());

		if(target.size() > 0)
			sendMessage(target.c_str(), messageFull);
		else
			sendMessage("", messageFull);
	} else {
		if(color)
			sprintf(messageFull, "%c%s%s: %s%c", 0x03, color, sender, msg.c_str(), 0x03);
		else
			sprintf(messageFull, "%s: %s", sender, msg.c_str());
		sendMessage("", messageFull);
	}
}

void IrcClient::parseIrcMessage(const std::string& message, std::string& prefix, std::string& command, std::string& parameters, std::string& trailing)
{
	size_t prefixEnd, trailingStart;
	size_t commandEnd;

	if (message[0] == ':') {
		prefixEnd = message.find_first_of(' ');
		prefix.assign(message, 1, prefixEnd - 1);
		prefixEnd++;
	} else {
		prefixEnd = 0;
	}

	trailingStart = message.find(" :");
	if (trailingStart != std::string::npos)
		trailing.assign(message, trailingStart+2, std::string::npos);
	else
		trailingStart = message.size();

	commandEnd = message.find_first_of(" \r\n", prefixEnd);
	if(commandEnd == std::string::npos)
		commandEnd = message.size();

	command.assign(message, prefixEnd, commandEnd - prefixEnd);
	parameters.assign(message, commandEnd, trailingStart - commandEnd);
}
