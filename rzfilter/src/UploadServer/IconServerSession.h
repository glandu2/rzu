#ifndef UPLOADSERVER_ICONSERVERSESSION_H
#define UPLOADSERVER_ICONSERVERSESSION_H

#include "SocketSession.h"
#include <string>
#include <sstream>

namespace UploadServer {

class IconServerSession : public SocketSession
{
	DECLARE_CLASS(UploadServer::IconServerSession)
public:
	IconServerSession();

	static bool checkName(const char* filename, size_t size);
	static const char* getAllowedCharsForName();

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

#endif // UPLOADSERVER_ICONSERVERSESSION_H
