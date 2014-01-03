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

#include "ClientData.h"
DECLARE_CLASSCOUNT_STATIC(ClientData)

#include "ClientInfo.h"
DECLARE_CLASSCOUNT_STATIC(ClientInfo)

#include "DB_Account.h"
DECLARE_CLASSCOUNT_STATIC(DB_Account)

#include "DesPasswordCipher.h"
DECLARE_CLASSCOUNT_STATIC(DesPasswordCipher)

#include "ServerInfo.h"
DECLARE_CLASSCOUNT_STATIC(ServerInfo)

#include "Upload/ClientInfo.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::ClientInfo)

#include "Upload/GameServerInfo.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::GameServerInfo)
