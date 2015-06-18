#include "LogServerClient.h"
#include "GlobalCoreConfig.h"
#include "Utils.h"
#include <string.h>

namespace AuthServer {

#pragma pack(push, 1)
struct LS_11N4S
{
	uint16_t id;
	uint16_t size;
	uint8_t type;
	uint32_t thread_id;

	int64_t n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11;

	uint16_t len1;
	uint16_t len2;
	uint16_t len3;
	uint16_t len4;
};
#pragma pack(pop)

LogServerClient* LogServerClient::instance = nullptr;

LogServerClient::LogServerClient(cval<std::string>& ip, cval<int>& port) :
	ip(ip), port(port)
{
	instance = this;
}

void LogServerClient::onConnected() {
	info("Connected to Log server %s:%d\n", ip.get().c_str(), port.get());

	sendLog(LM_SERVER_LOGIN, 0, 0, 0, Utils::getPid(), 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, "Main", NTS, CONFIG_GET()->app.appName.get().c_str(), NTS);

	sendLog(LM_SERVER_INFO, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, "START", -1);

	for(size_t i = 0; i < pendingMessages.size(); i++) {
		sendLog(pendingMessages[i]);
	}
	pendingMessages.clear();
}

void LogServerClient::stop() {
	sendLog(LM_SERVER_INFO, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, "END", -1);

	closeSession();
}

void LogServerClient::onDisconnected(bool causedByRemote) {
	info("Disconnected from Log server %s:%d\n", ip.get().c_str(), port.get());
}

void LogServerClient::sendLog(const Message& message) {
	LS_11N4S packet;
	packet.thread_id = message.thread_id;
	packet.type = message.type;
	packet.id = message.id;
	packet.n1 = message.n1;
	packet.n2 = message.n2;
	packet.n3 = message.n3;
	packet.n4 = message.n4;
	packet.n5 = message.n5;
	packet.n6 = message.n6;
	packet.n7 = message.n7;
	packet.n8 = message.n8;
	packet.n9 = message.n9;
	packet.n10 = message.n10;
	packet.n11 = message.n11;

	packet.len1 = (uint16_t) message.str1.size();
	packet.len2 = (uint16_t) message.str2.size();
	packet.len3 = (uint16_t) message.str3.size();
	packet.len4 = (uint16_t) message.str4.size();

	packet.size = sizeof(packet) + packet.len1 + packet.len2 + packet.len3 + packet.len4;

	instance->write(&packet, sizeof(packet));
	if(packet.len1)
		instance->write(&message.str1[0], packet.len1);
	if(packet.len2)
		instance->write(&message.str2[0], packet.len2);
	if(packet.len3)
		instance->write(&message.str3[0], packet.len3);
	if(packet.len4)
		instance->write(&message.str4[0], packet.len4);
}

void LogServerClient::sendLog(unsigned short id,
			 uint64_t n1,
			 uint64_t n2,
			 uint64_t n3,
			 uint64_t n4,
			 uint64_t n5,
			 uint64_t n6,
			 uint64_t n7,
			 uint64_t n8,
			 uint64_t n9,
			 uint64_t n10,
			 uint64_t n11,
			 const char * str1,
			 int len1,
			 const char * str2,
			 int len2,
			 const char * str3,
			 int len3,
			 const char * str4,
			 int len4)
{
	if(!instance || !instance->getStream() || instance->getStream()->getState() == Stream::UnconnectedState)
		return;

	Message message;

	message.id = id;
	message.thread_id = Utils::getPid();
	message.type = 1;

	message.n1 = n1;
	message.n2 = n2;
	message.n3 = n3;
	message.n4 = n4;
	message.n5 = n5;
	message.n6 = n6;
	message.n7 = n7;
	message.n8 = n8;
	message.n9 = n9;
	message.n10 = n10;
	message.n11 = n11;

	if(len1 == NTS && str1)
		len1 = (int) strlen(str1);

	if(len2 == NTS && str2)
		len2 = (int) strlen(str2);

	if(len3 == NTS && str3)
		len3 = (int) strlen(str3);

	if(len4 == NTS && str4)
		len4 = (int) strlen(str4);

	if(str1)
		message.str1.assign(str1, str1 + len1);
	if(str2)
		message.str2.assign(str2, str2 + len2);
	if(str3)
		message.str3.assign(str3, str3 + len3);
	if(str4)
		message.str4.assign(str4, str4 + len4);

	if(instance->getStream()->getState() == Stream::ConnectedState)
		instance->sendLog(message);
	else if(instance->pendingMessages.size() < 100)
		instance->pendingMessages.push_back(message);
}

} // namespace AuthServer
