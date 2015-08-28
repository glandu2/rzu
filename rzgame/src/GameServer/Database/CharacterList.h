#ifndef AUTHSERVER_DB_ACCOUNT_H
#define AUTHSERVER_DB_ACCOUNT_H

#include "Database/DbQueryJob.h"
#include <string>
#include <stdint.h>

class DbConnectionPool;
class DesPasswordCipher;

namespace GameServer {
class ClientSession;
}

namespace Database {

class CharacterList
{
public:
	struct Input {
		uint32_t account_id;
	};

	struct Output {
		uint32_t sid;
		char name[61];
		uint32_t race;
		uint32_t sex;
		uint32_t lv;
		uint32_t jlv;
		uint64_t exp;
		uint32_t hp;
		uint32_t mp;
		uint32_t job;
		uint32_t permission;
		uint32_t skin_color;
		uint32_t model_00;
		uint32_t model_01;
		uint32_t model_02;
		uint32_t model_03;
		uint32_t model_04;
		uint32_t hair_color_index;
		uint32_t hair_color_rgb;
		uint32_t hide_equip_flag;
		uint32_t texture_id;
		SQL_TIMESTAMP_STRUCT create_time;
		SQL_TIMESTAMP_STRUCT login_time;
		uint32_t login_count;
		SQL_TIMESTAMP_STRUCT logout_time;
		uint32_t play_time;
	};
};

} // namespace Database

#endif // AUTHSERVER_DB_ACCOUNT_H
