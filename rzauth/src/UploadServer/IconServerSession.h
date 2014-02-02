#ifndef GUILDICONSERVER_H
#define GUILDICONSERVER_H

#include "../SocketSession.h"
#include <string>
#include <sstream>

namespace UploadServer {

class IconServerSession : public SocketSession
{
	DECLARE_CLASS(UploadServer::IconServerSession)
public:
	IconServerSession();

protected:
	void onDataReceived();

	void parseData(const std::vector<char>& data);
	void parseUrl(std::string urlString);
	void sendIcon(const std::string& filename);

private:

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
