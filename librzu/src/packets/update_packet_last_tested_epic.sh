#!/bin/bash

TRAFFIC_LOG_FILE="$1"
TESTED_EPIC="$2"

if [ -z "$TRAFFIC_LOG_FILE" ] || [ -z "$TESTED_EPIC" ]; then
    echo "Usage: $0 traffic_log/rzfilter.log EPIC_9_8_1"
    exit 1
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

grep ': Packet json' "$TRAFFIC_LOG_FILE" | sed -r 's/.*: Packet json.*\((.*)\):/\1/' | sort -u |
while read -r packet; do
    for folder in AuthClient AuthGame GameClient UploadClient UploadGame; do
        PACKET_DEFINITION_FILE="${SCRIPT_DIR}/$folder/${packet}.h"
        if [ -f "$PACKET_DEFINITION_FILE" ]; then
            echo "Updating $PACKET_DEFINITION_FILE"
            if grep -q "// Last tested: " "$PACKET_DEFINITION_FILE"; then
                sed -i "s#// Last tested: .*#// Last tested: $TESTED_EPIC#" "$PACKET_DEFINITION_FILE"
            else
                gawk -i inplace -v "TESTED_EPIC=$TESTED_EPIC" 'FNR==NR{ if (/#include/) p=NR; next} 1; FNR==p{ print ""; print "// Last tested: " TESTED_EPIC }' "$PACKET_DEFINITION_FILE" "$PACKET_DEFINITION_FILE"
            fi
        fi
    done
done
