#!/bin/bash

source "../include/conf/usePCRE2.sh"
if $usePCRE2; then
    pcrelib="-lpcre2-8"
fi

if [ ! -d compiled ]; then
    mkdir compiled
fi

if [ ! -d "../../modules" ]; then
    mkdir "../../modules"
fi

sInstall=false

while [[ $1 == --* ]]; do
    if [[ "$1" == "--install" ]]; then
        sInstall=true
    fi
    shift 1
done

if [[ "$1" == "" ]]; then
    set -- *.cpp
fi

if [ ! -f "$1" ]; then
    echo "Nothing to compile . . ."
    exit 1
fi

while [ -f "$1" ]; do
    echo "Compiling '$1' . . ."
    src="$1"
    base=${src%.*}
    basename=${base##*/}
    fso="compiled/${basename}.fso"
    flags=""
    if [ -f "${base}.flags" ]; then
        flags=$(cat "${base}.flags" | tr -d '\n\r')
    fi
#    echo g++ -o "$fso" "$src" -I "../include" -L"../src/bin" -lfarage -shared -fPIC $pcrelib $flags
    g++ -o "$fso" "$src" -I "../include" -L"../src/bin" -lfarage -shared -fPIC $pcrelib $flags
    err=$?
    if [[ $err == 0 ]]; then
        if $sInstall; then
            cp "$fso" "../../modules/."
            if [[ $? == 0 ]]; then
                echo "Installed $base . . ."
            fi
        fi
    else
        echo "Error compiling '${src}': $err"
    fi
    shift 1
done

