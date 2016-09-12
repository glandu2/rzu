try {
	Start-Process .\rzauth -ArgumentList '/configfile:auth-test.opt'
	Start-Sleep -s 1
	.\rzauth_test /test.spawnexec=false /core.log.level:trace /core.log.consolelevel:info
	if($LastExitCode -ne 0) {
		throw "rzauth test failed: $lastexitcode"
	}
	Start-Process .\rzgamereconnect -ArgumentList '/auth.reconnectdelay:100'
	Start-Sleep -s 1
	.\rzauth_test /test.spawnexec=false /core.log.level:trace /core.log.consolelevel:info /auth.game.port:4802 --gtest_filter=-TS_GA_LOGIN_WITH_LOGOUT.*
	if($LastExitCode -ne 0) {
		throw "rzauth reconnect test failed: $lastexitcode"
	}
	#Send-StringOverTcp "terminate`n" "127.0.0.1" "4501"
	.\rzterminator /port:4501
	.\rzgamereconnect_test /test.spawnexec=false /core.log.level:trace /core.log.consolelevel:info
	if($LastExitCode -ne 0) {
		throw "rzgamereconnect test failed: $lastexitcode"
	}
} finally {
	.\rzterminator /port:4501
	.\rzterminator /port:4801
}
