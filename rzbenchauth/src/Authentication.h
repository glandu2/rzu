#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "Object.h"
#include "IListener.h"
#include <string>
#include <stdint.h>
#include <IDelegate.h>
#include "Packets/PacketEnums.h"
#include <vector>
#include "uv.h"
#include "DesPasswordCipher.h"

class Account;
class RappelzSocket;
struct TS_MESSAGE;
struct TS_AC_SERVER_LIST;
struct TS_AC_SELECT_SERVER;
struct TS_AC_AES_KEY_IV;
struct TS_SC_RESULT;

class Authentication : private Object, IListener
{
	DECLARE_CLASSNAME(Authentication, 0)
	public:
		struct ServerInfo {
			uint16_t serverId;
			std::string serverName;
			std::string serverScreenshotUrl;
			std::string serverIp;
			int32_t serverPort;
			uint16_t userRatio;
		};
		enum AuthCipherMethod {
			ACM_DES,
			ACM_RSA_AES  //Since mid epic 8.1
		};

		typedef void (*CallbackOnAuthResult)(IListener* instance, Authentication* auth, TS_ResultCode result, const std::string& resultString);
		typedef void (*CallbackOnServerList)(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId);
		typedef void (*CallbackOnGameResult)(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket);
		typedef void (*CallbackOnAuthClosed)(IListener* instance, Authentication* auth);

	public:
		Authentication(std::string host, AuthCipherMethod method = ACM_DES, uint16_t port = 4500, const std::string& version = "200701120");
		~Authentication();

		int connect(Account* account, const std::string& password, Callback<CallbackOnAuthResult> callback);
		void abort(Callback<CallbackOnAuthClosed> callback);

		bool retreiveServerList(Callback<CallbackOnServerList> callback);
		bool selectServer(uint16_t serverId, Callback<CallbackOnGameResult> callback);

		const std::string& getAccountName() { return username; }

		int index; //for main.cpp

	protected:
		static void onAuthServerConnectionEvent(IListener* instance, RappelzSocket *server, const TS_MESSAGE* packetData);
		static void onGameServerConnectionEvent(IListener* instance, RappelzSocket* server, const TS_MESSAGE* packetData);
		static void onAuthPacketReceived(IListener* instance, RappelzSocket* server, const TS_MESSAGE* packetData);
		static void onGamePacketReceived(IListener* instance, RappelzSocket* server, const TS_MESSAGE* packetData);

	protected:
		void onPacketAuthConnected();
		void onPacketAuthClosed();
		void onPacketAuthUnreachable();
		void onPacketAuthPasswordKey(const TS_AC_AES_KEY_IV *packet);
		void onPacketServerList(const TS_AC_SERVER_LIST* packet);
		void onPacketSelectServerResult(const TS_AC_SELECT_SERVER* packet);
		void onPacketGameConnected();
		void onPacketGameUnreachable();
		void onPacketGameAuthResult(const TS_SC_RESULT* packet);

	private:
		struct ServerConnectionInfo {
			uint16_t id;
			std::string ip;
			uint16_t port;
		};

	private:
		RappelzSocket* authServer, *gameServer;
		std::string authIp;
		uint16_t authPort;
		AuthCipherMethod cipherMethod;
		std::string version;
		std::string username;
		std::string password;
		unsigned char aes_key_iv[32];
		void* rsaCipher;
		std::vector<ServerConnectionInfo> serverList;
		int selectedServer;
		uint64_t oneTimePassword;
		bool inProgress;
		static DesPasswordCipher desCipher;

		Callback<CallbackOnAuthResult> authResultCallback;
		Callback<CallbackOnGameResult> gameResultCallback;
		Callback<CallbackOnServerList> serverListCallback;
		Callback<CallbackOnAuthClosed> authClosedCallback;
};

#endif // AUTHENTICATION_H
