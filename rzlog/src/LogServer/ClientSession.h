#pragma once

#include "LogPacketSession.h"
#include <sql.h>

#include "LS_11N4S.h"

namespace LogServer {

class UploadRequest;

class ClientSession : public LogPacketSession {
	DECLARE_CLASS(LogServer::ClientSession)

public:
	ClientSession();

	struct LogData {
		SQL_TIMESTAMP_STRUCT date;
		uint32_t thread_id;
		uint16_t id;

		int64_t n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11;
		std::string s1, s2, s3, s4;
	};

protected:
	EventChain<LogPacketSession> onPacketReceived(const LS_11N4S* packet);

	bool isDateNeedNewFile(const struct tm& date);
	bool checkName(std::string name);
	void updateOpenedFile(const tm& date);

private:
	~ClientSession();
	FILE* file;
	std::string serverName;
	struct {
		uint16_t year;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
	} fileDate;
};

}  // namespace LogServer

