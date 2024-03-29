#include "DbTypeHelpers.h"
#include "Core/Utils.h"
#include <stdio.h>

void DbDateTime::setUnixTime(time_t t, uint32_t nanoFraction) {
	struct tm date;
	Utils::getGmTime(t, &date);

	year = date.tm_year;
	month = date.tm_mon;
	day = date.tm_mday;
	hour = date.tm_hour;
	minute = date.tm_min;
	second = date.tm_sec;
	fraction = nanoFraction;
}

time_t DbDateTime::getUnixTime() const {
	struct tm date;

	date.tm_year = year;
	date.tm_mon = month;
	date.tm_mday = day;
	date.tm_hour = hour;
	date.tm_min = minute;
	date.tm_sec = second;

	return Utils::getTimeGm(&date);
}

void DbPrintableTypeBinding<SQL_TIMESTAMP_STRUCT>::print(std::ostringstream& stream, void* data) {
	SQL_TIMESTAMP_STRUCT* date = reinterpret_cast<SQL_TIMESTAMP_STRUCT*>(data);
	char buffer[128];

	snprintf(buffer,
	         sizeof(buffer),
	         "%04d-%02d-%02d %02d:%02d:%012.9f",
	         date->year,
	         date->month,
	         date->day,
	         date->hour,
	         date->minute,
	         date->second + ((double) date->fraction) / 1000000000.0);

	stream << buffer;
}
