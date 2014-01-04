#ifndef GUILDICONSERVER_H
#define GUILDICONSERVER_H

#include "Object.h"
#include "Socket.h"
#include "ICallbackGuard.h"
#include <string>
#include <sstream>

namespace UploadServer {

class GuildIconServer : public Object, public ICallbackGuard
{
	DECLARE_CLASS(UploadServer::GuildIconServer)
public:
	GuildIconServer(Socket* socket);
	~GuildIconServer();

	static void startServer();

protected:
	static void onNewConnection(ICallbackGuard* instance, Socket* serverSocket);
	static void onStateChanged(ICallbackGuard* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState);
	static void onDataReceived(ICallbackGuard *instance, Socket* socket);

	void parseUrl();
	void sendIcon(const std::string& filename);

private:
	Socket* socket;

	bool retrievingUrl;
	std::ostringstream url;
};

} // namespace UploadServer

#endif // GUILDICONSERVER_H
