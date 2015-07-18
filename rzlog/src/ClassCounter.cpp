#include "ClassCounter.h"

//generated with notepad++ with this pattern: (.*) ->
//#include "\1.h"\nDECLARE_CLASSCOUNT_STATIC\(\1\)\n

#include "AdminServer/AdminInterface.h"
DECLARE_CLASSCOUNT_STATIC(AdminServer::AdminInterface)

#include "LogServer/ClientSession.h"
DECLARE_CLASSCOUNT_STATIC(LogServer::ClientSession)

#include "LogServer/LogPacketSession.h"
DECLARE_CLASSCOUNT_STATIC(LogServer::LogPacketSession)
