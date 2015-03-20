#ifndef AUTHSERVER_LOGSERVERCLIENT_H
#define AUTHSERVER_LOGSERVERCLIENT_H

#include "SocketSession.h"
#include "StartableObject.h"
#include "ConfigParamVal.h"

namespace AuthServer {

class LogServerClient : public SocketSession, public StartableObject
{
public:
	LogServerClient(cval<std::string>& ip, cval<int>& port);

	enum MessageType {
		LM_SERVER_LOGIN = 101,
		LM_SERVER_INFO = 102,
		LM_SERVER_WARNING = 103,
		LM_SERVER_STATUS = 104,

		LM_ACCOUNT_LOGIN = 1001,
		LM_ACCOUNT_LOGOUT = 1002,
		LM_ACCOUNT_DUPLICATE_AUTH_LOGIN = 1003,
		LM_ACCOUNT_DUPLICATE_GAME_LOGIN = 1004,
		LM_GAME_SERVER_LOGIN = 1101,
		LM_GAME_SERVER_LOGOUT = 1102
	};

	struct Message {
		uint16_t id;
		uint8_t type;
		uint32_t thread_id;

		int64_t n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11;
		std::vector<char> str1;
		std::vector<char> str2;
		std::vector<char> str3;
		std::vector<char> str4;
	};

	static const int NTS = -1;

	bool start() { return connect(ip.get().c_str(), port.get()); }
	void stop();
	bool isStarted() { return getStream() && getStream()->getState() == Stream::ConnectedState; }

	void onConnected();
	void onDisconnected(bool causedByRemote);

	void sendLog(const Message& message);

	static void sendLog(unsigned short id,
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
				 int len4);

private:
	static LogServerClient* instance;
	cval<std::string>& ip;
	cval<int>& port;

	std::vector<Message> pendingMessages;
};

} // namespace AuthServer

#endif // AUTHSERVER_LOGSERVERCLIENT_H
