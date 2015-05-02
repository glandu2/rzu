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

#include "AdminServer/AdminInterface.h"
DECLARE_CLASSCOUNT_STATIC(AdminServer::AdminInterface)

#include "AuthServer/BillingInterface.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::BillingInterface)

#include "AuthServer/ClientData.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::ClientData)

#include "AuthServer/ClientSession.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::ClientSession)

#include "AuthServer/DB_Account.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::DB_Account)

#include "AuthServer/DB_SecurityNoCheck.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::DB_SecurityNoCheck)

#include "AuthServer/DB_UpdateLastServerIdx.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::DB_UpdateLastServerIdx)

#include "AuthServer/GameData.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::GameData)

#include "AuthServer/GameServerSession.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::GameServerSession)

#include "UploadServer/ClientSession.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::ClientSession)

#include "UploadServer/GameServerSession.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::GameServerSession)

#include "UploadServer/IconServerSession.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::IconServerSession)

#include "UploadServer/UploadRequest.h"
DECLARE_CLASSCOUNT_STATIC(UploadServer::UploadRequest)
