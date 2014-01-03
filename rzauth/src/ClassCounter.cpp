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

#include "AuthServer/ClientInfo.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::ClientInfo)

#include "AuthServer/DB_Account.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::DB_Account)

#include "AuthServer/ServerInfo.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::ServerInfo)

#include "UploadServer/ClientInfo.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::ClientInfo)

#include "UploadServer/GameServerInfo.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::GameServerInfo)
