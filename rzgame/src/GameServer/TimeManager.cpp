#include "TimeManager.h"
#include "Core/Utils.h"

namespace GameServer {

rztime_t TimeManager::getRzTime() {
    static uint64_t baseTime = Utils::getTimeInMsec();
    return static_cast<uint32_t>((Utils::getTimeInMsec() - baseTime) / 10);
}

}
