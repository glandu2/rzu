#!/bin/sh

cd librzu/src && find . -name '*.h' | python ../../generateGuards.py
cd ../../rzauth/src && find . -name '*.h' | python ../../generateGuards.py
cd ../../rzbenchauth/src && find . -name '*.h' | python ../../generateGuards.py
cd ../../rzgame/src && find . -name '*.h' | python ../../generateGuards.py
cd ../../PlayerCount/src && find . -name '*.h' | python ../../generateGuards.py
cd ../../ChatGateway/src && find . -name '*.h' | python ../../generateGuards.py
cd ../../VStructGen/src && find . -name '*.h' | python ../../generateGuards.py

