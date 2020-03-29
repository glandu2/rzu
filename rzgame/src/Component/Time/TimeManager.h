#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include "GameTypes.h"
#include <stdint.h>

namespace GameServer {

typedef ar_time_t rztime_t;  // unit [10ms] since first call
class ClientSession;

class TimeManager {
public:
	static rztime_t getRzTime();
	static void sendGameTime(ClientSession* session);
};

}  // namespace GameServer

#endif  // TIMEMANAGER_H
