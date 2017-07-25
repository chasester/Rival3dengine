#!/bin/bash

export client_exec="rival_client"
export server_exec="rival_server"
export master_exec="rival_master"
#need enet here? o_O
export ignore_dirs="documentation|extralib|include|lib|lib64|licenses|vcpp|win_login|xcode"

get() {
    [ ! ${!1} ] && echo "Empty arg passed to ${FUNCNAME[0]}! Exiting." && exit 1
    find . -name *${1} | egrep -v "$ignore_dirs"
}

return_client_objs() {
    get ".cpp" | sed 's%\./%%g ; s/cpp/o/g'
}

return_file_dependencies() {
    local file=$1
    [ ! ${!file} ] && echo "Empty arg passed to ${FUNCNAME[0]}! Exiting." && exit 1
    [ ! -f ${file} ] && echo "File $file not found on disk! Exiting." && exit 1
    echo "$1"
}

return_gcc_flags() {
    echo
}

return_linker_flags() {
    echo
}

clean() {
    printf ">>> Removing object files..."
    rm -f $(get ".o")
    echo " Done"

    printf ">>> Removing precompiled header object files..."
    rm -f $(get ".h.gch")
    echo " Done"

    printf ">>> Removing client/server/master binaries from source/..."
    rm -f $client_exec $server_exec $master_exec
    echo " Done"
}

argtypes="/clean/build/install/"

usage(){
    echo "Usage: $0 $argtypes"
}

#return_client_objs
#return_file_dependencies


#what we need to do:

#Get the list of source files that need to be built
#Auto-discover dependencies of that file (needed for linking later)
#
#
#Thoughts:
#       Maybe generating the dep list is a lot easier than we think? Just find all the header files and use that in the linker command to each source file? :D
#going the route of generating the GCC calls will be a LOT easier than trying to generate a Makefile :D
#oh, we need that progress bar.... :D
# we need to make a separate command stdout file for make install, make server and make master
# we should have our own customized make clean commnad

[ $# -eq 0 ] && usage && exit 1
[ ! "$(echo $argtypes | grep /$1/)" ] && usage && exit 1
$1


