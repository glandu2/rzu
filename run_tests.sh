#!/bin/sh

nc_location=$(which nc)
if [ -z "$nc_location" ]; then
	nc_location="../nc.exe"
fi
./rzauth /configfile:auth-test.opt 2> /dev/null &
sleep 1
./rzauth_test --gtest_repeat=2
if [ $? -ne 0 ]; then
	echo terminate | $nc_location 127.0.0.1 4501
	exit
fi
./rzgamereconnect /auth.reconnectdelay:100 2> /dev/null &
sleep 1
./rzauth_test /game.port:4802 --gtest_repeat=2 --gtest_filter=-TS_GA_LOGIN_WITH_LOGOUT.*
if [ $? -ne 0 ]; then
	echo terminate | $nc_location 127.0.0.1 4501
	echo terminate | $nc_location 127.0.0.1 4801
	exit
fi
echo terminate | $nc_location 127.0.0.1 4501
./rzgamereconnect_test --gtest_repeat=2
echo terminate | $nc_location 127.0.0.1 4801
