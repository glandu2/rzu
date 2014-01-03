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

#include "Auth/ClientData.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::ClientData)

#include "Auth/ClientInfo.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::ClientInfo)

#include "Auth/DB_Account.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::DB_Account)

#include "Auth/ServerInfo.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::ServerInfo)

#include "Upload/ClientInfo.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::ClientInfo)

#include "Upload/GameServerInfo.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::GameServerInfo)
