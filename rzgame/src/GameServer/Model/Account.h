#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "Character.h"
#include <unordered_map>

namespace GameServer {

class ClientSession;

class Player {
public:
private:
	std::unique_ptr<Character> character;
	ClientSession* session;
};

}

#endif // ACCOUNT_H
