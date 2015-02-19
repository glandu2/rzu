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

#include "TelnetSession.h"
DECLARE_CLASSCOUNT_STATIC(TelnetSession)

#include "AuthServer/AuthSession.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::AuthSession)

#include "AuthServer/ClientData.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::ClientData)

#include "AuthServer/GameServerSession.h"
DECLARE_CLASSCOUNT_STATIC(AuthServer::GameServerSession)
