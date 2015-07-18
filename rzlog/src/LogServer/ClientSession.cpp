#include "ClientSession.h"
#include "../GlobalConfig.h"
#include <time.h>
#include "Utils.h"
#include <time.h>
#include <string.h>

namespace LogServer {

ClientSession::ClientSession() {
	file = nullptr;
	memset(&fileDate, 0, sizeof(fileDate));
}

ClientSession::~ClientSession() {
	if(file)
		fclose(file);
}

bool ClientSession::isDateNeedNewFile(const struct tm& date) {
	if(date.tm_year == fileDate.year &&
			date.tm_mon == fileDate.month &&
			date.tm_mday == fileDate.day &&
			date.tm_hour == fileDate.hour)
		return false;
	else
		return true;
}

void ClientSession::updateOpenedFile(const struct tm& date) {
	if(!serverName.empty() && (file == nullptr || isDateNeedNewFile(date))) {
		if(file)
			fclose(file);

		fileDate.year = date.tm_year;
		fileDate.month = date.tm_mon;
		fileDate.day = date.tm_mday;
		fileDate.hour = date.tm_hour;

		char filename[512];
		sprintf(filename,
				"%s_%04d-%02d-%02d %02d.txt",
				serverName.c_str(),
				fileDate.year,
				fileDate.month,
				fileDate.day,
				fileDate.hour);

		std::string absoluteDir = CONFIG_GET()->log.logDir.get();
		std::string fullFileName = absoluteDir + "/" + filename;

		file = fopen(fullFileName.c_str(), "wt");
	}
}

bool ClientSession::checkName(std::string name) {
	const char* p = name.c_str();
	size_t size = name.size();

	for(size_t i = 0; i < size; i++) {
		const char c = p[i];

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

void ClientSession::onPacketReceived(const LS_11N4S* packet) {
	debug("Received log packet id %d, size %d\n", packet->id, packet->size);
	LogData logData;

	const int expectedSize = sizeof(*packet) + packet->len1 + packet->len2 + packet->len3 + packet->len4;
	if(expectedSize > packet->size) {
		error("Invalid packet size, got %d, expected %d\n", packet->size, expectedSize);
		return;
	}

	struct tm date;
	uint64_t timemsec = Utils::getTimeInMsec();

	Utils::getGmTime(timemsec/1000, &date);

	logData.date.year = date.tm_year;
	logData.date.month = date.tm_mon;
	logData.date.day = date.tm_mday;
	logData.date.hour = date.tm_hour;
	logData.date.minute = date.tm_min;
	logData.date.second = date.tm_sec;
	logData.date.fraction = (timemsec % 1000) * 1000000; // unit: ns

	logData.id = packet->id;
	logData.thread_id = packet->thread_id;

	logData.n1 = packet->n1;
	logData.n2 = packet->n2;
	logData.n3 = packet->n3;
	logData.n4 = packet->n4;
	logData.n5 = packet->n5;
	logData.n6 = packet->n6;
	logData.n7 = packet->n7;
	logData.n8 = packet->n8;
	logData.n9 = packet->n9;
	logData.n10 = packet->n10;
	logData.n11 = packet->n11;

	const char* p = reinterpret_cast<const char*>(packet) + sizeof(*packet);
	logData.s1 = Utils::convertToString(p, packet->len1);
	p += packet->len1;
	logData.s2 = Utils::convertToString(p, packet->len2);
	p += packet->len2;
	logData.s3 = Utils::convertToString(p, packet->len3);
	p += packet->len3;
	logData.s4 = Utils::convertToString(p, packet->len4);

	if(logData.id == 101) {
		if(!serverName.empty()) {
			warn("Already received login message for server %s. Received new name is %s (ignored)\n", serverName.c_str(), logData.s4.c_str());
		} else if(!checkName(logData.s4)) {
			error("Received login message with invalid characters in server name: %s\n", logData.s4.c_str());
		} else {
			serverName = logData.s4;
		}
	}

	updateOpenedFile(date);

	if(file) {
		static char lineBuffer[20 + 20*13 + 255*4 + 18];
		int len = sprintf(lineBuffer,
						  "%04d-%02d-%02d %02d:%02d:%02d.%03d\t"
						  "%d\t%d\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t"
						  "%s\t%s\t%s\t%s\n",
						  logData.date.year,
						  logData.date.month,
						  logData.date.day,
						  logData.date.hour,
						  logData.date.minute,
						  logData.date.second,
						  (int)(timemsec % 1000),

						  logData.thread_id,
						  logData.id,
						  logData.n1,
						  logData.n2,
						  logData.n3,
						  logData.n4,
						  logData.n5,
						  logData.n6,
						  logData.n7,
						  logData.n8,
						  logData.n9,
						  logData.n10,
						  logData.n11,

						  logData.s1.c_str(),
						  logData.s2.c_str(),
						  logData.s3.c_str(),
						  logData.s4.c_str());

		fwrite(lineBuffer, len, 1, file);
	}
}

} // namespace LogServer
