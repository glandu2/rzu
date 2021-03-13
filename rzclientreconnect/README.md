# About

This tool allows to keep a client running even if the server is disconnected.
This can be useful in case of bad network condition, or when being sometimes kicked from the GS.

# Usage

* Use the config template `rzclientreconnect.opt`
* Replace `server.ip` with the auth server's IP
* Replace `server.account` and `server.password` with account/password to use (see comments in the template file)
* Set `server.gsname` with the GS server name
* Set `server.playername` with the character to connect to
* If needed, adjust `general.recoDelay` with the period between reconnection in seconds

* Start reclientreconnect

* Start the client (never use bora command line arguments as a classic connection will be sufficient and faster)
  * Use a command line like this: `RappelzCmdLauncher.exe SFrame.exe /auth_ip:127.0.0.1 /locale:windows-1252 /country:FR`

* When logging in in the client, use a random account and password, everything will be accepted as the config
  in the .opt file will be used instead to connect to the real auth server

# Command available in game

rzclientreconnect implements several commands via the in game chat.
To use them, use a private message to "rzcr" fake player.

Example:  `"rzcr help`

Available commands (arguments between `<>` are required, `[]` or optional):

* help
  * List available commands
* add <account name> <password> <character>
  * Connect a seccondary character on the same GS using given account/password
* remove <character>
  * Disconnect a secondary character by its name
* list
  * List all connected character by their name
* switch [character name]
  * Switch the current character in the client to another connected character
  * If no character name is given, the command try to use the current character's target

