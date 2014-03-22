#include "ClassCounter.h"

//generated with notepad++ with this pattern: (.*) ->
//#include "\1.h"\nDECLARE_CLASSCOUNT_STATIC\(\1\)\n
/* Class list:
ClientData
ClientInfo
DB_Account
DesPasswordCipher
ServerInfo
*/

#include "AuthServer/ClientData.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::ClientData)

#include "AuthServer/ClientSession.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::ClientSession)

#include "AuthServer/DB_Account.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::DB_Account)

#include "AuthServer/GameServerSession.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::GameServerSession)

#include "RappelzServer.h"
DECLARE_CLASSCOUNT_STATIC(RappelzServerCommon)

#include "RappelzSession.h"
DECLARE_CLASSCOUNT_STATIC(RappelzSession)

#include "SocketSession.h"
DECLARE_CLASSCOUNT_STATIC(SocketSession)

#include "UploadServer/ClientSession.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::ClientSession)

#include "UploadServer/GameServerSession.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::GameServerSession)

#include "UploadServer/IconServerSession.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::IconServerSession)
