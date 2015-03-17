#include <iostream>
#include "Packets/TS_AC_SERVER_LIST.h"

/* Tests:
 * TestPacketServer => Test, name
 * TestPacketSession => TestPacketServer
 * Test: onPacketReceived(TestPacketSession, std::string sourceName / enum, const Packet)
 * ProcessSpawner => config => name / enum (used to generate pipe name)
 */
