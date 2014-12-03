#include "IconServerSession.h"
#include "../GlobalConfig.h"
#include <string.h>
#include <stdio.h>
#include "Utils.h"

namespace UploadServer {

static const char * const htmlNotFound =
		"HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 22\r\n"
		"\r\n"
		"<h1>404 Not Found</h1>";
static int htmlNotFoundSize = strlen(htmlNotFound);

static const char * const htmlFound =
		"HTTP/1.1 200 Ok\r\n"
		"Content-Type: image/jpeg\r\n"
		"Content-Length: %ld\r\n"
		"\r\n";
static int htmlFoundSize = strlen(htmlFound);

IconServerSession::IconServerSession() {
	this->status = WaitStatusLine;
	this->nextByteToMatch = 0;
	this->urlLength = 0;
}

void IconServerSession::onDataReceived() {
	std::vector<char> buffer;

	if(getStream()->getAvailableBytes() > 0) {
		getStream()->readAll(&buffer);
		parseData(buffer);
	}
}

void IconServerSession::parseData(const std::vector<char>& data) {
	static const char * const beginUrl = "GET ";
	static const char * const endHeader = "\r\n\r\n";
	const char* begin = &data[0];
	const char* end = &data[0] + data.size();

	for(const char* p = begin; p < end; p++) {
		if(status == WaitStatusLine) {
			if(*p == beginUrl[nextByteToMatch]) {
				nextByteToMatch++;
				if(nextByteToMatch >= 4) {
					status = RetrievingStatusLine;
					nextByteToMatch = 0;
				}
			} else {
				nextByteToMatch = 0;
			}
		} else if(status == RetrievingStatusLine) {
			if(*p == '\r' || *p == '\n') {
				status = WaitEndOfHeaders;
			} else if(*p >= 32 && *p <= 126 && urlLength < 255) {
				url.put(*p);
				urlLength++;
			} else {
				status = WaitStatusLine;
				nextByteToMatch = 0;
				url.str(std::string());
				url.clear();
				urlLength = 0;
			}
		}

		if(status == RetrievingStatusLine || status == WaitEndOfHeaders) {
			if(*p == endHeader[nextByteToMatch]) {
				nextByteToMatch++;
				if(nextByteToMatch >= 4) {
					status = WaitStatusLine;
					nextByteToMatch = 0;

					std::string urlString = url.str();
					if(!urlString.compare(urlString.size() - 9, std::string::npos, " HTTP/1.1")) {
						urlString.resize(urlString.size() - 9);
						parseUrl(urlString);
					}

					url.str(std::string());
					url.clear();
					urlLength = 0;
				}
			} else {
				nextByteToMatch = 0;
			}
		}
	}
}

void IconServerSession::parseUrl(std::string urlString) {
	ssize_t p;
	for(p = urlString.size()-1; p >= 0; p--) {
		const char c = urlString.at(p);
		if(c == '/' || c == '\\' || c == ':')
			break;
	}
	if(p+1 >= (ssize_t)urlString.size()) {
		//attempt to get a directory
		getStream()->write(htmlNotFound, htmlNotFoundSize);
	} else {
		std::string filename = urlString.substr(p+1, std::string::npos);
		if(checkName(filename.c_str(), filename.size())) {
			sendIcon(filename);
		} else {
			warn("Request to a invalid filename: \"%s\"\n", filename.c_str());
			getStream()->write(htmlNotFound, htmlNotFoundSize);
		}
	}
}

bool IconServerSession::checkName(const char* filename, size_t size) {
	for(size_t i = 0; i < size; i++) {
		const char c = filename[i];

		//Update getAllowedCharsForName when changing this condition
		if(!((c >= '0' && c <= '9') ||
			 (c >= 'A' && c <= 'Z') ||
			 (c >= 'a' && c <= 'z') ||
			 c == '_' || c == '.'))
		{
			return false;
		}
	}

	return true;
}

const char* IconServerSession::getAllowedCharsForName() {
	return "0-9, A-Z, a-z, _, and .";
}

void IconServerSession::sendIcon(const std::string& filename) {
	std::string fullFileName = CONFIG_GET()->upload.client.uploadDir.get() + "/" + filename;
	FILE* file = fopen(fullFileName.c_str(), "rb");

	if(!file) {
		getStream()->write(htmlNotFound, htmlNotFoundSize);
	} else {
		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);
		if(fileSize > 64000)
			fileSize = 64000;

		int bufferSize = htmlFoundSize + 10 + fileSize;
		char *buffer = new char[bufferSize];
		size_t fileContentBegin = sprintf(buffer, htmlFound, (long int)fileSize);

		size_t bytesTransferred = 0;
		size_t nbrw = 0;
		while(bytesTransferred < fileSize) {
			nbrw = fread(buffer + fileContentBegin + bytesTransferred, 1, fileSize - bytesTransferred, file);
			if(nbrw <= 0)
				break;
			bytesTransferred += nbrw;
		}

		fclose(file);

		if(nbrw > 0) {
			getStream()->write(buffer, fileContentBegin + fileSize);
		} else {
			getStream()->write(htmlNotFound, htmlNotFoundSize);
		}

		delete[] buffer;
	}
}

} //namespace UploadServer
