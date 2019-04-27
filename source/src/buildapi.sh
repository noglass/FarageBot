#!/bin/bash

if [ ! -d "bin" ]; then
    mkdir "bin"
fi

curDir=$PWD
cd ../..
git fetch &>/dev/null
official=$(git rev-list --count HEAD)
status="$(git status)
Untracked files:"
status=$(echo "$status" | grep -B 999 -m 1 '^Untracked files:')
branch=$(echo "$status" | grep -E '^On branch' | sed 's/^On branch master$//; s/^On branch \(.*\)$/\1 /;')
modified=$(echo "$status" | grep -E '^Changes not staged for commit:$' | sed 's/^Changes not staged for commit:$/ \(modified\)/')
revision="${branch}Revision: ${official}${modified}"
apimod=$(echo "$status" | grep -E '\b(include/api/|src/api/)')
if [[ ${#apimod} -gt 0 ]]; then
    apimod=true
else
    apimod=false
fi
cd "$curDir"

origversion=$(cat ../include/conf/.farageAPIbuild.h | tr -d '\n\r' | sed 's/[^\"]*\"\([^\"]*\)\".*/\1/')
if [[ $origversion == *build-* ]]; then
    oldversion=${origversion#*build-}
fi
if $apimod; then
    version=$(echo $oldversion | awk '{ ver=$1 + 0.001;printf "%.3f", ver }')
else
    version=$oldversion
fi
echo "#define FARAGE_API_VERSION      \"${revision} build-${version}\"" > ../include/conf/.farageAPIbuild.h

if [[ -f "../include/conf/usePCRE2.sh" ]]; then
    source "../include/conf/usePCRE2.sh"
fi
succ=0
if $usePCRE2; then
    echo "#define FARAGE_USE_PCRE2" > ../include/conf/regex_style.h
    if ! [[ -f "bin/pcre2_halfwrap.o" ]]; then
        g++ -std=c++11 -o bin/pcre2_halfwrap.o -c shared/pcre2_halfwrap.cpp -I "../include/shared" -fPIC
        succ=$?
    fi
else
    echo "" > ../include/conf/regex_style.h
    if [[ -f "bin/pcre2_halfwrap.o" ]]; then
        rm bin/pcre2_halfwrap.o
    fi
fi
if [[ $succ -eq 0 ]]; then
    g++ -std=c++11 -c api/*.cpp -I "../include" -ldl -fPIC "$@" && mv *.o bin/. && ar rvs bin/libfarage.a bin/*.o
    succ=$?
fi
if [[ $succ -ne 0 ]]; then
    echo "#define FARAGE_API_VERSION      \"$origversion\"" > ../include/conf/.farageAPIbuild.h
    exit 1
else
    echo Successfully compiled FarageAPI v$version
fi
