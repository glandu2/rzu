#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <stdint.h>

namespace GameServer {

typedef uint32_t rztime_t; // unit [10ms] since first call
class ClientSession;

class TimeManager
{
public:
	static rztime_t getRzTime();
	static void sendGameTime(ClientSession* session);
};

}

#endif // TIMEMANAGER_H
