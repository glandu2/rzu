#include "ClassCounter.h"

#include "GameServer/AuthServerSession.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::AuthServerSession)

#include "GameServer/ClientSession.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::ClientSession)

#include "GameServer/ConnectionHandler/LobbyHandler.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::LobbyHandler)

#include "GameServer/ConnectionHandler/PlayerLoadingHandler.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::PlayerLoadingHandler)
