Function Send-StringOverTcp ($DataToSend, $hostname, $port) {
    Try
    {
        $ErrorActionPreference = "Stop"
        $TCPClient  = New-Object Net.Sockets.TcpClient
        $TCPClient.Connect($hostname, $port)
        $NetStream  = $TCPClient.GetStream()
        [Byte[]]$Buffer = [Text.Encoding]::ASCII.GetBytes($DataToSend)
        $NetStream.Write($Buffer, 0, $Buffer.Length)
        $NetStream.Flush()
		while($NetStream.ReadByte() -ne -1) {}
    } catch {}
    Finally
    {
        If ($NetStream) { $NetStream.Dispose() }
        If ($TCPClient) { $TCPClient.Dispose() }
    }
}

try {
	Start-Process .\rzauth -ArgumentList '/configfile:auth-test.opt'
	Start-Sleep -s 1
	.\rzauth_test /core.log.level:trace /core.log.consolelevel:info
	if($LastExitCode -ne 0) {
		throw "rzauth test failed: $lastexitcode"
	}
	Start-Process .\rzgamereconnect -ArgumentList '/auth.reconnectdelay:100'
	Start-Sleep -s 1
	.\rzauth_test /core.log.level:trace /core.log.consolelevel:info /auth.game.port:4802 --gtest_filter=-TS_GA_LOGIN_WITH_LOGOUT.*
	if($LastExitCode -ne 0) {
		throw "rzauth reconnect test failed: $lastexitcode"
	}
	Send-StringOverTcp "terminate`n" "127.0.0.1" "4501"
	.\rzgamereconnect_test /core.log.level:trace /core.log.consolelevel:info
	if($LastExitCode -ne 0) {
		throw "rzgamereconnect test failed: $lastexitcode"
	}
} finally {
	Send-StringOverTcp "terminate`n" "127.0.0.1" "4501"
	Send-StringOverTcp "terminate`n" "127.0.0.1" "4801"
}
