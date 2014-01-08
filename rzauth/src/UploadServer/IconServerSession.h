#ifndef GUILDICONSERVER_H
#define GUILDICONSERVER_H

#include "Object.h"
#include "Socket.h"
#include "IListener.h"
#include <string>
#include <sstream>

namespace UploadServer {

class IconServerSession : public Object, public IListener
{
	DECLARE_CLASS(UploadServer::IconServerSession)
public:
	IconServerSession(Socket* socket);
	~IconServerSession();

	static void startServer();

protected:
	static void onNewConnection(IListener* instance, Socket* serverSocket);
	static void onStateChanged(IListener* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState);
	static void onDataReceived(IListener *instance, Socket* socket);


	void parseData(const std::vector<char>& data);
	void parseUrl(std::string urlString);
	void sendIcon(const std::string& filename);

private:
	Socket* socket;

	enum State : char {
		WaitStatusLine,
		RetrievingStatusLine,
		WaitEndOfHeaders
	} status;

	uint8_t nextByteToMatch;

	std::ostringstream url;
	uint8_t urlLength;
};

} // namespace UploadServer

#endif // GUILDICONSERVER_H
