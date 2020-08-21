Table of contents
=================

 * [Introduction](#introduction)
 * [Features](#features)
 * [Usage guide](#usage-guide)
 * [Client IMBC autologin](#imbc)


Introduction
============

This is an Auth Server emu ready for production usage.
It handle player connections, authentication and game server selection.

It support many database (default configuration is for SQL Server), clients/GS since epic 2 (running fine with 5.2 GS and newer).

It is portable on Windows and Linux.

Database support
----------------
This server support any database that have an ODBC driver, this include:

 * SQL Server (supported by all Windows OS out of the box)
 * [Oracle](http://www.oracle.com/technetwork/database/features/instant-client/index-097480.html)
 * [MySql](http://dev.mysql.com/downloads/connector/odbc/)
 * [PostgreSQL](http://www.postgresql.org/ftp/odbc/versions/msi/)
 * [SQLite](http://www.ch-werner.de/sqliteodbc/)

For some of them you will need a correct connection string to give parameters to configure the connection. Connection string for almost all known database are available here: http://www.connectionstrings.com/ (for example, to get the connection string to a SQLite database, type "sqlite connection string odbc" in google, click on the first link and look for the driver you have in "ODBC drivers").

The list of available ODBC drivers can be retrieved with this tool:
On windows x64: C:\WINDOWS\SysWOW64\odbcad32.exe
On windows x86: C:\WINDOWS\System32\odbcad32.exe


Features
========

The server has most of the feature of the official auth server with some additional features:

 * Auth server using RSA/AES or DES
 * Auth server using [IMBC autologin](#imbc)
 * Auto server reconnect to SQL database on failure
 * Guild icon server with an integrated mini webserver (can serve only JPG images)
 * Network traffic dump (not enabled by default)
 * Support epic 2 to latest
 * Support GS reconnection without GS restart with [GS autoreconnect](https://github.com/glandu2/rzgamereconnect/) (in case of bad network connection between auth and GS or auth restart)
 * Support security password (for bank access, character delete, ...)
 * Optionnaly encrypted database password/connection string (both old DES and 9.1 encryption)
 * Send log to [Log Server](https://github.com/glandu2/rzlog/)

Account character restriction note:
-----
By default, original auth server restrict which characters that can be used.
This restriction is enabled in this auth server too, thus authentication with accounts with other characters than letters or digits are refused.
Be sure that your sign up page does not allow other characters than these for the account name.

You can disable this restriction using `auth.clients.restrictchars` (see also in the [Usage guide](#usage-guide)).

Usage guide
===========

Configuration
-------------

To use the auth server, you must have a correct configuration. For that two methods are available:

 * Use command line switches
 * Use a configuration file

For example, changing the ip of the Database server you need to change the variable `auth.db.server`


Command line switches
---------------------

Command line switches use this formats:
```
/<variable_name>:<value>
```

`/` can be replaced by `-` and `:` can be replaced by `=`.

This method override values set in the configuration file.
The configuration file location can be changed using this method with the argument `/configfile:filename.opt`.

Example:
```
rzauth.exe /configfile:auth_for_epic82.opt /admin.console.autostart:true /admin.console.port:6235 /auth.db.salt:2011
```

Configuration file
------------------

By default, the configuration file is named `auth.opt` (This can be overridden by changing `configfile` value using the command line)

The file has one configuration value per line with empty lines or lines beginning with the `#` character ignored (this allows comments)
A configuration file has this format:
```
<variable_name>:<value>
<variable_name>=<value>
```


The separator is `:` and `=`. Any spaces are treated as the value or name of the variable (including trailing spaces, only leading spaces before the variable name are ignored)

It's possible to include another config file using the special command `%include`.
This following command will include configuration from `common.opt` file:
```
%include common.opt
```

Example of configuration file:

```sh
#Database configuration
auth.db.server:127.0.0.1
auth.db.port:1433
auth.db.name:Auth
auth.db.account:sa

#MD5 salt
auth.db.salt:2011
#The password is in plain text for auth.db.password if you have one
auth.db.password:
#Use auth.db.cryptedpassword instead of auth.db.password to use an encrypted password
auth.db.cryptedpassword:<password encrypted using Pyrok tool>

#Enable traffic dump, will be in traffic_log folder (default off)
#trafficdump.enable:true

#Where clients will connect (this is default values)
auth.clients.ip:0.0.0.0
auth.clients.port:4500

#Where the gameserver will connect (this is default values)
auth.gameserver.ip:127.0.0.1
auth.gameserver.port:4502

#Upload configuration, use "upload" folder for guild icons
upload.dir=upload
upload.clients.ip=0.0.0.0
upload.clients.port=4617
#Use port 5000 for the guild icon mini webserver
upload.iconserver.ip=0.0.0.0
upload.iconserver.port:5000

#Where the gameserver will connect (this is default values)
upload.gameserver.ip:127.0.0.1
upload.gameserver.port:4616

#Only for SQL Server: speed up selects with NOLOCK
sql.db_account.query:SELECT * FROM account WITH(NOLOCK) WHERE account = ? AND password = ?;

#Configure security password query. If this query return at least one row, the security password is accepted
#sql.db_securitynocheck.query:SELECT securty_no FROM account WHERE account = ? AND securty_no = ?;
#You can also use a stored procedure
#sql.db_securitynocheck.query:{CALL smp_new_check_security(?, ?)}

#Logging level
core.log.level:debug
core.log.consolelevel:info
```

Telnet interface
----------------

The telnet-like interface is the way to interact with the server.
This allows:

 * Change or display a configuration value
 * Start or stop a server component (like start the auth client server when all game servers are connected)


This way it's possible to change a server port or change the log level without restarting the emu.

When you connect to the admin server, it shows this:
```
Auth server - Administration server - Type "help" for a list of available commands
>
```

After every command, when the server waits for a new command, it sends the command prompt `> `.

The available commands are (also available with the "help" command):

 * `get <variable_name>`
   Show the value of the variable.
 * `set <variable_name> <value>`
   Set a new value for the variable.
 * `start <server_name>`
   Start the server server_name.
   If `<server_name>` is "all", then start all server with their variable "autostart" set to "true".
 * `stop <server_name>`
   Stop the server server_name. If <server_name> is "all" then stop all servers (this will cause the emu to terminate).
 * `list`
   List all connected gameservers and infos about them: index, name, IP address, players count on it, screenshot url
 * `mem`
   List emu's objects counts.
 * `closedb`
   Close all idle DB connections. Use this to bring the auth database offline without stopping the auth server.
 * `help`
   Show the list of available commands with a description (like this list).
 * `terminate`
   Stop the emu (equivalent to `stop all`).


In any case, when all servers are stopped (either by using `stop all` or by stopping all servers one by one), the emu will terminate.

Available server names are:

 * `auth.clients`
   The auth server listening for clients
 * `auth.gameserver`
   The auth server listening for game servers
 * `auth.billing`
   The billing telnet server listening for billing notifications
 * `auth.logclient`
   The Log Server connection to send activity logs
 * `upload.clients`
   The upload server listening for clients for guild icon uploads
 * `upload.iconserver`
   The web server listening for clients to download guild icons
 * `upload.gameserver`
   The upload server listening for game servers
 * `admin.console`
   The telnet administration server used to interact with the Emu

To connect to the telnet server, you can use [PuTTY](http://the.earth.li/~sgtatham/putty/latest/x86/putty.exe). To connect to it, use `127.0.0.1` as hostname, `4501` as the port and use `RAW mode` (4501 is the default port, see configuration variable `admin.console.port`). Change values as needed for your configuration.


Configuration variables
-----------------------

The format used to display configuration variable information is the same between the startup dump and the result of the `get` command on a telnet interface.

The format is:
```
<type><is_default><variable_name>:<value>
```

`<type>` can be:

 * B : Boolean
 * N : Number (integer)
 * F : Float
 * S : String


`<is_default>` can be either a space or a star.
If there is a star, the value is the default one.
The default value can depend on other configuration variable, for example `auth.db.connectionstring` use other `auth.db.*` values.

Example of output:

	B*auth.gameserver.autostart:true
	S*auth.gameserver.ip:127.0.0.1
	N auth.gameserver.port:5014


Variables list
--------------

### Database configuration
All configuration variables about the database live under `auth.db`.

Variable|Type|Description|Default value
--------|----|-----------|-------------
auth.db.account|String|The account to use to connect to the database|sa
auth.db.connectionstring|String|The full connection string. If other configuration values are not enough to configure the ODBC driver, use this, else leave it with default value. For information about connection strings, see there: [ConnectionStrings.com](http://www.connectionstrings.com/)|The default value is based on other values in auth.db
auth.db.cryptedconnectionstring|String|Encrypted connection string. Use Pyrok's tool to encrypt a string. This config take precedence over `auth.db.connectionstring`|<nothing>
auth.db.driver|String|The ODBC driver name. Tell which type of database to use, should rarely be changed|*SQL Server* on Windows (installed by default since Windows XP), [*FreeTDS*](https://packages.debian.org/jessie/tdsodbc) on Linux
auth.db.ignoreinitcheck|Boolean|If true, then ignore failure when testing the connection to the database at startup. If false, a failure of the DB test will cause the emu to terminate|true
auth.db.name|String|The database name, usually Auth|Auth
auth.db.password|String|The password to use to connect to the database|<nothing>
auth.db.cryptedpassword|String|Encrypted password. Use Pyrok's tool to encrypt a string. This config take precedence over `auth.db.password`|<nothing>
auth.db.port|Integer|The port of the database server|1433
auth.db.salt|String|The salt to use to check a player's password. The MD5 hash is created like this: <salt><password> (for example: "2011password" if the password of the player is "password" and the salt is "2011"|2011
auth.db.server|String|The database server ip|127.0.0.1
auth.securityno.salt|String|The salt to use to check security no (see `auth.db.salt`)|2011


### Auth server configuration
This configure how clients and gameservers must connect to the auth server.
All configuration variables about the auth server live under `auth.clients` and `auth.gameserver`.

Variable|Type|Description|Default value
--------|----|-----------|-------------
auth.clients.autostart|Boolean|If true, the auth client server will listen for clients automatically at startup. If false you will need the telnet server and type `start auth.clients` to start listening for clients|true
auth.clients.des_key|String|The DES key to use for the authentication method prior to 8.1. This should never be changed unless you know what you do|MERONG
auth.clients.enableimbc|Boolean|If true, IBMC login is enabled|true
auth.clients.idletimeout|Integer|If a connection to the auth client server is idle for more than this value (in seconds), it will be kicked. The real kick occurs between this value and 2x this value. If set to 0, idle connections are never kicked|301 (5min 1s)
auth.clients.ip|String|The interface IP to listen on. Use 0.0.0.0 for all interface or the IP of one network interface to listen on that interface|0.0.0.0
auth.clients.maxpublicserveridx|Integer|The default maximum gameserver's index for public servers shown in client's server list. Use with `server_idx_offset` column in Account table. If `server_idx_offset + auth.clients.maxpublicserveridx > gameserver's index`, the gameserver is not shown in the client's server list. `server_idx_offset` column can be renamed with the config value `sql.db_account.column.serveridxoffset`|30
auth.clients.port|Integer|The port to listen on for clients|4500
auth.clients.restrictchars|Boolean|If true, restrict account characters to letters and digits only|true
auth.gameserver.autostart|Boolean|If true, the server will listen for gameservers automatically at startup. If false you will need the telnet server and type `start auth.gameserver` to start listening for gameservers|true
auth.gameserver.idletimeout|Integer|If a connection from a gameserver to the auth server is idle for more than this value (in seconds), it will be kicked. The real kick occurs between this value and 2x this value. If set to 0, idle connections are never kicked|0
auth.gameserver.ip|String|The interface IP to listen on. Use 0.0.0.0 for all interface or the IP of one network interface to listen on only one interface|127.0.0.1
auth.gameserver.maxplayers|Integer|An indicator for the maximum supported players on one gameserver. Used only for the GS load color shown in the client's server list|400
auth.gameserver.port|Integer|The port to listen on for gameservers|4502
auth.gameserver.strictkick|Boolean|Used for duplicate login kick. If false, an account is considered as disconnected even if the GS does not reply to kick request|true

#### Gameserver hidding
About hidding gameservers for devs only or for a closed beta server:
If you want:

 * All servers with index <= 10 to be public (by default, anyone can see them),
 * All servers with index between 11 and 20 (included) to be used for closed tests (ie: closed beta) with access for only a limited amount of players,
 * All servers with index between 21 to 50 for tests purpose with only access for devs


Then use these config values:
Set `auth.clients.maxpublicserveridx` to 10
Default value for the `server_idx_offset` columns in Account table is set to 0
Players that can access closed beta test gameservers have their `server_idx_offset` column set to 10 (they can access all servers with `index <= auth.clients.maxpublicserveridx + server_idx_offset`, so `index <= 10+10`
Set `server_idx_offset` column for Devs to 40 so they can access all servers with `index <= 10 + 40`, so `index <= 50`

The `server_idx_offset` column in the Account table can be renamed using the config parameter `sql.db_account.column.serveridxoffset`.

### Log Server configuration
This configure the connection to the Log Server.
Operation like account connection, disconnection or kick is logged to the Log Server.

Variable|Type|Description|Default value
--------|----|-----------|-------------
logclient.enable|Boolean|If true, the emu will connect to the Log Server and send activity logs. If false you will need the telnet server and type `start auth.logclient` to connect to the Log Server|true
logclient.ip|String|The IP of the Log Server|127.0.0.1
logclient.port|Integer|The port of the Log Server|4516


### Billing telnet notification server configuration
This configure the telnet server accepting `billing_notify blank <account_id>` commands.
When the emu receive such command, it will send a notification to the game server so the player will have a notification in its chat window about having received a new item from item shop.

Variable|Type|Description|Default value
--------|----|-----------|-------------
auth.billing.autostart|Boolean|If true, the billing telnet server will listen for notifications automatically at startup. If false you will need the telnet server and type `start auth.billing` to start listening|true
auth.billing.idletimeout|Integer|If a connection to the billing telnet server is idle for more than this value (in seconds), it will be kicked. The real kick occurs between this value and 2x this value. If set to 0, idle connections are never kicked|0
auth.billing.ip|String|The IP to listen on for billing notifications|127.0.0.1
auth.billing.port|Integer|The port to listen on for billing notifications|4503

### Upload server configuration
This configure the upload server.
All configuration variables about the upload server live under `upload`.

Variable|Type|Description|Default value
--------|----|-----------|-------------
upload.clients.autostart|Boolean|If true, the upload client server will listen for clients automatically at startup. If false you will need the telnet server and type `start upload.clients` to start listening for clients|true
upload.clients.idletimeout|Integer|If a client connection to the upload server is idle for more than this value (in seconds), it will be kicked. The real kick occurs between this value and 2x this value. If set to 0, idle connections are never kicked|61 (1min 1s)
upload.clients.ip|String|The interface IP to listen on. Use 0.0.0.0 for all interface or the IP of one network interface to listen on only one interface|0.0.0.0
upload.clients.port|Integer|The port to listen on for clients|4617
upload.iconserver.autostart|Boolean|If true, the guild icon mini webserver will listen for clients automatically at startup. If false you will need the telnet server and type `start upload.iconserver` to start the mini webserver|true
upload.iconserver.idletimeout|Integer|If a client connection to the mini webserver is idle for more than this value (in seconds), it will be kicked. The real kick occurs between this value and 2x this value. If set to 0, idle connections are never kicked|31 (31s)
upload.iconserver.ip|String|The interface IP to listen on. Use 0.0.0.0 for all interface or the IP of one network interface to listen on only one interface|0.0.0.0
upload.iconserver.port|Integer|The webserver port to listen on for clients. Clients will connect to this port to download the icon file. Port numbers lower than 1024 require the server to be started with admin privileges on some OS (like Linux)|80
upload.gameserver.autostart|Boolean|If true, the server will listen for gameservers automatically at startup. If false you will need the telnet server and type "start upload.gameserver" to start listening for gameservers|true
upload.gameserver.idletimeout|Integer|If a gameserver connection to the upload server is idle for more than this value (in seconds), it will be kicked. The real kick occurs between this value and 2x this value. If set to 0, idle connections are never kicked|0
upload.gameserver.ip|String|The interface IP to listen on. Use 0.0.0.0 for all interface or the IP of one network interface to listen on only one interface|127.0.0.1
upload.gameserver.port|Integer|The port to listen on for gameservers|4616
upload.dir|String|The directory where to put guild icon files. If the directory is not an absolute path, it is relative to where the Emu is|upload


The value of `app.name` of game servers must contains only letters (a-z and A-Z), digits (0-9), _ or -

Here is an example of guild icon configuration for the gameserver.opt:
```
S game.guild_icon_base_url:/
S game.url_list:guild_icon_upload.ip|< auth emu external IP >|guild_icon_upload.port|< upload.clients.port >|guild_test_download.url|upload/|web_download|< auth emu external IP >:< upload.clients.webport >
```

### SQL queries configuration
This configure which SQL query to do for the auth server.
All configuration variables about SQL queries live under `sql`.

Variable|Type|Description|Default value
--------|----|-----------|-------------
sql.db_account.column.accountid|String|The name of the column containing the account ID. If this column does not exist, every authentication will fail|account_id
sql.db_account.column.age|String|The name of the column containing the age. The default value if this column does not exist is "19"|age
sql.db_account.column.authok|String|The name of the column containing the auth result. The default value if this column does not exist is "1"|auth_ok
sql.db_account.column.block|String|The name of the column containing the ban flag. The default value if this column does not exist is "0"|block
sql.db_account.column.eventcode|String|The name of the column containing the event code. The default value if this column does not exist is "0"|event_code
sql.db_account.column.lastserveridx|String|The name of the column containing the last game server the player connected to. The default value if this column does not exist is "1"|last_login_server_idx
sql.db_account.column.password|String|The name of the column containing the password value. If this column does not exist, the authentication is accepted as long as a row is returned by the query, auth_ok = 1 and block = 0. Else the password is also checked|password
sql.db_account.column.pcbang|String|The name of the column containing the PCBang value. The default value if this column does not exist is "0"|pcbang
sql.db_account.column.serveridxoffset|String|The name of the column containing the max public server index offset value. The default value if this column does not exist is "0"|server_idx_offset
sql.db_account.enable|Boolean|If false, this query is not executed and every authentication will fail. A query that fail more than 10 times in a row will be automatically disabled (use the telnet admin interface to reenable it using "set sql.db_account.enable true"). Failed connections to the DB are not counted|true
sql.db_account.param.account|Integer|The index of the "?" in the query that contains the account name to connect (the index of the first "?" is 1)|1
sql.db_account.param.password|Integer|The index of the "?" in the query that contains the password provided by the client (the index of the first "?" is 1)|2
sql.db_account.param.ip|Integer|The index of the "?" in the query that contains the client's ip (the index of the first "?" is 1)|-1
sql.db_account.query|String|The query to execute. Use "?" character for account and password parameters|SELECT * FROM account WHERE account = ? AND password = ?;
sql.db_updatelastserveridx.enable|Boolean|If false, this query is not executed and last used gameserver index won't be updated. A query that fail more than 10 times in a row will be automatically disabled (use the telnet admin interface to reenable it using "set sql.db_updatelastserveridx.enable true"). Failed connections to the DB are not counted|true
sql.db_updatelastserveridx.param.serveridx|Integer|The index of the "?" in the query that contains the game server index to set (the index of the first "?" is 1)|1
sql.db_updatelastserveridx.param.accountid|Integer|The index of the "?" in the query that contains the account id to update (the index of the first "?" is 1)|2
sql.db_updatelastserveridx.query|String|The query to execute. Use "?" character for account id and game server index parameters|UPDATE account SET last_login_server_idx = ? WHERE account_id = ?;
sql.db_securitynocheck.enable|Boolean|If false, this query is not executed and all security password will be refused. A query that fail more than 10 times in a row will be automatically disabled (use the telnet admin interface to reenable it using "set sql.db_securitynocheck.enable true"). Failed connections to the DB are not counted|true
sql.db_securitynocheck.param.account|Integer|The index of the "?" in the query that contains the account for which the security password must be checked (the index of the first "?" is 1)|1
sql.db_securitynocheck.param.securityno|Integer|The index of the "?" in the query that contains the security password to be checked (the index of the first "?" is 1)|2
sql.db_securitynocheck.query|String|The query to execute. by default, the security password is the same as the account password. Use "?" character for account id and game server index parameters|SELECT account FROM account WHERE account = ? AND password = ?;


It's possible to use stored procedures instead of SQL queries.
For example, to use official stored procedures:
```
sql.db_account.param.account:1
sql.db_account.param.ip:2
sql.db_account.param.password:3
sql.db_account.query:{CALL smp_account(?, ?, ?)}
sql.db_updatelastserveridx.param.accountid:1
sql.db_updatelastserveridx.param.serveridx:2
sql.db_updatelastserveridx.query:{CALL smp_update_last_login_server_idx(?, ?)}
sql.db_securitynocheck.param.account:1
sql.db_securitynocheck.param.securityno:2
sql.db_securitynocheck.query:{CALL smp_check_security(?, ?)}
```

### Administration configuration
All configuration variables about the telnet server live under `admin`.

Variable|Type|Description|Default value
--------|----|-----------|-------------
admin.dump_mode|Integer|Configure crash dump. (Windows only). Set to 1 to disable crash dump generation on a crash. The crashdump contains the state of the server when a crash occurs. Set to any other value to activate crash dump generation. A crash dump usually is less than 1MB and is saved as crashdump.dmp|0
admin.console.autostart|Boolean|If true, the telnet server will be started when the Emu start. If set to false, the telnet server will not start disabling the telnet access (there is no way to start it manually after, as the telnet administration will be disabled)|true
admin.console.idletimeout|Integer|If a connection to the telnet server is idle for more than this value (in seconds), it will be kicked. The real kick occurs between this value and 2x this value. If set to 0, idle connections are never kicked|0
admin.console.ip|String|The interface where to listen telnet client. If you put 0.0.0.0, it will accept ANY IP even from internet, so use with care. The telnet server is meant to be the replacement for the console integrated in the official auth and thus should listen on 127.0.0.1 only (local connections only)|127.0.0.1
admin.console.port|Integer|The port for the telnet server to listen on|4501


### Miscellaneous variables

Variable|Type|Description|Default value
--------|----|-----------|-------------
ban.ipfile|String|The filename that list banned IPs. The file must have one IP per line in the form of: x.x.x.x|bannedip.txt
configfile|String|The configuration filename. Only the command line has an effect, it's used to change the config file to use|auth.opt
core.appname|String|The application name. Provided only for information purpose and is currently not used anywhere|rzauth
core.config.showhidden|Boolean|When starting, the configuration is dumped on the console. This variable controls whether to dump sensitive variables like passwords. If false, sensitive variables won't be printed|false
core.encoding|String|Encoding to use for strings from the client to/from a database|CP1252
core.log.consolelevel|String|The log level for the console. Available values are: "fatal", "error", "warning", "info", "debug" and "trace"|<value of core.log.level>
core.log.dir|String|The directory where to put log files|log
core.log.enable|Boolean|If true, logging is enabled. Log messages are written to the file `<core.log.dir>/<core.log.file>_YYYY-MM-DD.log`. If false, no log is output to file and on console|true
core.log.file|String|The base filename of the log file. Real filename will have the date appended to its name|auth.log
core.log.level|String|The log level. The level is used for the file and for the console if core.log.consolelevel has not been set to another value. Available values are: "fatal", "error", "warning", "info", "debug" and "trace"|info
core.log.maxqueuesize|Integer|The log message queue size. Larger number means more memory used when there are many log message to write to disk. If there are too many log message, they are discarded|10000
core.stream_cipher|String|The RC4 cipher to be used in communication between the client and the server|
core.usetcpnodelay|Boolean|If true, all connections will use TCP_NODELAY attribute|false
trafficdump.consolelevel|String|The log level for the console. Available values are the same as core.log.consolelevel. This should be left to "fatal" (traffic dump is very verbose)|fatal
trafficdump.dir|String|The directory where to put traffic dump files|traffic_log
trafficdump.enable|Boolean|If true, traffic dump is enabled. Data is written to the file `<trafficdump.dir>/<trafficdump.file>_YYYY-MM-DD.log`. If false, no traffic dump is done|true
trafficdump.file|String|The base filename of the log file. Real filename will have the date appended to its name|auth.log
trafficdump.level|String|The log level. The level is used for the file and for the console if core.log.consolelevel has not been set to another value. Connection state changes have the "info" level and data dump has the "debug" level|debug
trafficdump.dump_raw|Boolean|When the trafficdump is enabled, this control whether to dump packets in raw hexdump format. In older versions, only this format was available for trafficdump. This has no effect if the trafficdump is not enabled|false
trafficdump.dump_json|Boolean|When the trafficdump is enabled, this control whether to dump packets in JSON format. This has no effect if the trafficdump is not enabled|true


### Informative variables

Variable|Type|Description|Default value
--------|----|-----------|-------------
core.version|String|Tell the version of the core library (librzu.dll if provided in a separated file) and the build date (UTC)|Depends on the build
global.version|String|Tell the version of the Auth Emu server and the build date (UTC)|Depends on the build
stats.connections|Integer|The number of connections opened by clients or game servers|0
stats.disconnections|Integer|The number of disconnections opened by clients or game servers|0

To get the current opened connections to the Emu, calculate stats.connections - stats.disconnections or use "mem" command and check for the number of "Socket".


Client IMBC autologin
=====================

IMBC autologin allow to start SFrame.exe with an account and password set in the command line. So the client does not ask for account & password when started, but directly authenticate the provided account in the command line.

The SFrame command line to use to enable this feature is this one:
```
/imbclogin /account:<account_name> /password:<password>
```

**/!\ The password is in plain text. Be aware that the command line used to run an application is visible to any other program running on your computer.**
This feature should be used along with a Launcher that does the player authentication and generate One Time passwords.

The idea is to make the password passed to SFrame.exe usable only once. To do that, you probably need to change the auth emu query to use another SQL table:
```
sql.db_account.query:SELECT * FROM one_time_passwords WHERE account = ? AND password = ?;
```

And all rows in the `one_time_passwords` table would be temporary. The one time passwords need to be hashed like a normal password, that is using MD5 and a salt like "2011".


Note about Visual C++ Runtime
=============================

If Windows complains about missing msvcr100.dll or msvcp100.dll, install the [Visual C++ 2010 runtime redistributable x86](http://www.microsoft.com/en-us/download/details.aspx?id=5555).

