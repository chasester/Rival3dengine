#!/bin/bash

[ $# -lt 2 ] && {
    echo "Takes two args: source/master file and file being checked"
    exit 1
}

[ -f $1 ] && [ -f $2 ] || {
    echo "File specified not found on disk, retry"
    exit 1
}

for line in $(cat $1); {
    #echo "Testing: \"$line\""
    [ "$(grep $line $2)" ] ||
        echo "\"$line\" not found in $2.."
}
