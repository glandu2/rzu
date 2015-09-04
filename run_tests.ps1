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
	Start-Process .\rzauth -ArgumentList  '/configfile:auth-test.opt'
	.\rzauth_test --gtest_repeat=2
	if(-not $?) {
		throw "rzauth test failed"
	}
	Start-Process .\rzgamereconnect
	.\rzauth_test /game.port:4802 --gtest_filter=-TS_GA_LOGIN_WITH_LOGOUT.*
	if(-not $?) {
		throw "rzauth reconnect test failed"
	}
	Send-StringOverTcp "terminate`n" "127.0.0.1" "4501"
	.\rzgamereconnect_test
	if(-not $?) {
		throw "rzgamereconnect test failed"
	}
} finally {
	Send-StringOverTcp "terminate`n" "127.0.0.1" "4501"
	Send-StringOverTcp "terminate`n" "127.0.0.1" "4801"
}
