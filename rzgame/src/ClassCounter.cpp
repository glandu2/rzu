#include "ClassCounter.h"

#include "AdminServer/AdminInterface.h"
DECLARE_CLASSCOUNT_STATIC(AdminServer::AdminInterface)

#include "GameServer/AuthServerSession.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::AuthServerSession)

#include "GameServer/ClientSession.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::ClientSession)
