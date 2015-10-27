#!/bin/sh

cd librzu/src && find . -name '*.h' | python ../../generateGuards.py
cd ../../rzauth/src && find . -name '*.h' | python ../../generateGuards.py
cd ../../rzbenchauth/src && find . -name '*.h' | python ../../generateGuards.py
cd ../../rzgame/src && find . -name '*.h' | python ../../generateGuards.py
cd ../../rzplayercount/src && find . -name '*.h' | python ../../generateGuards.py
cd ../../rzchatgateway/src && find . -name '*.h' | python ../../generateGuards.py
