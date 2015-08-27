@echo off
start rzauth /configfile:auth-test.opt
rzauth_test --gtest_repeat=2
IF ERRORLEVEL 1 (
	echo terminate | %~dp0\nc 127.0.0.1 4501
	GOTO end
)
start rzgamereconnect
rzauth_test /game.port:4802 --gtest_repeat=2 --gtest_filter=-TS_GA_LOGIN_WITH_LOGOUT.*
IF ERRORLEVEL 1 (
	echo terminate | %~dp0\nc 127.0.0.1 4501
	echo terminate | %~dp0\nc 127.0.0.1 4801
	GOTO end
)
echo terminate | %~dp0\nc 127.0.0.1 4501
rzgamereconnect_test --gtest_repeat=2
echo terminate | %~dp0\nc 127.0.0.1 4801
:end