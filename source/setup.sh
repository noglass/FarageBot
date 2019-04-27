#!/bin/bash

# Setup script version 1.1

chmod +x src/buildapi.sh
chmod +x modules/build.sh

curDir=$PWD

git &> /dev/null
if [[ $? == 127 ]]; then
    echo "git is not installed! You will not be able to download necessary dependencies!"
    echo -n "Continue anyway? [Y/n] "
    read ans
    if [[ $ans == y* ]] || [[ $ans == Y* ]]; then
        nogit=true
    else
        exit 1
    fi
else
    nogit=false
fi
wget &> /dev/null
if [[ $? == 127 ]]; then
    curl &> /dev/null
    if [[ $? == 127 ]]; then
        echo "Error: Either wget or curl is required to download dependencies."
        echo -n "Continue anyway? [Y/n]: "
        read ans
        if [[ $ans == y* ]] || [[ $ans == Y* ]]; then
            download=false
        else
            exit 1
        fi
    else
        download="curl -o "
    fi
else
    download="wget -O "
fi

echo "This script will setup everything you need to get FarageBOT up and running."
echo ""
if [ ! -d ../config ]; then
    mkdir ../config
    mkdir ../config/priority
elif [ ! -d ../config/priority ]; then
    mkdir ../config/priority
fi
doConf=false
if [ ! -f ../config/farage.conf ]; then
    doConf=true
else
    echo -n "Would you like to re-create the runtime configuration now? [Y/n] "
    read ans
    echo ""
    if [[ $ans == y* ]] || [[ $ans == Y* ]]; then
        doConf=true
    fi
fi
if $doConf; then
    echo "Setting up runtime configuration . . ."
    echo ""
    echo '[farage]
verbose=true
debug=true' > ../config/farage.conf
    echo -n "Do you have a bot token yet? [Y/n] "
    read ans
    if [[ $ans == y* ]] || [[ $ans == Y* ]]; then
        echo "Please enter your bot token and press enter:"
        read ans
        echo ""
        echo "token=${ans}" >> ../config/farage.conf
    else
        echo ""
        echo "token=YOUR BOT TOKEN HERE" >> ../config/farage.conf
        echo 'A placeholder has been added to the "../config/farage.conf" file. You will need to manually enter you bot token before running faragebot.'
        echo ""
    fi
    echo 'Enter the default command prefix you would like your bot to use, default is "!":'
    read ans
    echo "prefix=${ans}" >> ../config/farage.conf
    echo ""
    echo "Runtime configuration complete."
    echo 'These settings can be changed by editing the "../config/farage.conf" file at any time.'
    echo ""
fi
if [ ! -f ../config/admins.conf ]; then
    echo '# Here you can set globally active admin flags.
# Unlike the adminroles.ini file, admins defined here will
# have power everywhere including within Direct Messages.
# 
# It is a good idea to at least define your own ID here.
# 
# Format:
# 
# <USER_ID> = <flags>
# 
# Example:
# 
# 153786881912340480 = z # nigel
# 
# You can use a # to mark a line as a comment.
# You can also place a # in the middle of a line to mark
# everything from that point on that line as a comment.' > ../config/admins.conf
    echo -n "Would you like to setup the admin config file now? [Y/n] "
    read ans
    if [[ $ans == y* ]] || [[ $ans == Y* ]]; then
        echo "Enter your own discord user ID and press enter: "
        read ans
        echo ""
        echo "${ans} = z # This is me!" >> ../config/admins.conf
        echo "You are now set as a root admin."
        echo ""
        echo -n "Would you like to add more admins now? [Y/n] "
        read ans
        echo ""
        while [[ $ans == y* ]] || [[ $ans == Y* ]]; do
            echo "Enter the discord user ID of the admin you would like to create:"
            read ans
            echo ""
            echo "Enter the flags you would like to give this admin:"
            read flags
            echo ""
            echo "${ans} = ${flags}" >> ../config/admins.conf
            echo -n "Would you like to add another admin now? [Y/n] "
            read ans
            echo ""
        done
    else
        echo 'The default "../config/admins.conf" file has been created complete with instructions for adding admins.'
        echo "No admins have been defined."
    fi
    echo 'You can change the admin config by editing the "../config/admin.conf" file at any time.'
    echo ""
fi

setup=false
echo -n "Do you have SleepyDiscord already? [Y/n] "
read ans
echo ""
if [[ $ans == y* ]] || [[ $ans == Y* ]]; then
    while [ 1 ]; do
        echo "Please input the full path where the git repository is installed:"
        read sleepypath
        echo ""
        if [[ -d "$sleepypath" ]]; then
            break
        else
            echo "Error: Invalid path."
        fi
    done
else
    if $nogit; then
        echo "Cannot continue without git installed!"
        exit 1
    fi
    sleepypath=$PWD/sleepy-discord/
    git clone https://github.com/yourWaifu/sleepy-discord.git
    setup=true
    echo ""
fi

echo '#!/bin/bash
curDir=$PWD' > build.tmp
echo -n 'cd ../..
git fetch &>/dev/null
official=$(git rev-list --count HEAD)
status="$(git status)
Untracked files:"
status=$(echo "$status" | grep -B 999 -m 1 ' >> build.tmp
echo "'^Untracked files:')" >> build.tmp
echo -n 'mod=$(echo "$status" | grep -E ' >> build.tmp
echo "'\b(include/(api|engine)/|src/(api|engine)/)')" >> build.tmp
echo 'if [[ ${#mod} -gt 0 ]]; then
    mod=true
else
    mod=false
fi' >> build.tmp
echo "cd \"$sleepypath\"" >> build.tmp
echo 'git fetch &>/dev/null
sleepyrevision=$(git rev-list --count HEAD)' >> build.tmp
echo -n 'sleepyversion=$(git status | grep -E ' >> build.tmp
echo -n "'^(On branch .*|Changes not staged for commit:)" >> build.tmp
echo -n '$' >> build.tmp
echo -n "' | sed " >> build.tmp
echo -n '"s/^On branch \(.*\)\$/\1 Revision: $sleepyrevision/" | sed ' >> build.tmp
echo -n "'s/^Changes not staged for commit:" >> build.tmp
echo -n '$/\(modified\)/' >> build.tmp
echo "')" >> build.tmp
echo -n 'cd "$curDir"
sleepyversion=$(echo SleepyDiscord-$sleepyversion)
echo Compiling FarageBOT with $sleepyversion
echo "#define SLEEPY_VERSION \"$sleepyversion\"" > ../include/conf/sleepyversion.h

apiversion=$(cat ../include/conf/.farageAPIbuild.h | tr -d ' >> build.tmp
echo "'\n\r' | sed 's/[^\\\"]*\\\"\\([^\\\"]*\\)\\\".*/\\1/')" >> build.tmp
echo -n 'echo Using FarageAPI v$apiversion

oldengversion=$(cat ../include/conf/.farageEngineBuild.h | tr -d ' >> build.tmp
echo "'\n\r' | sed 's/[^\\\"]*\\\"\\([^\\\"]*\\)\\\".*/\\1/')" >> build.tmp
echo -n 'if $mod; then
    engversion=$(echo $oldengversion | awk ' >> build.tmp
echo -n "'{ ver=" >> build.tmp
echo -n '$1 + 0.001;printf "%.3f", ver }' >> build.tmp
echo "')" >> build.tmp
echo '    echo "#define FARAGE_ENGINE           \"$engversion\"" > ../include/conf/.farageEngineBuild.h
else
    engversion=$oldengversion
fi

if [[ -f "../include/conf/usePCRE2.sh" ]]; then
    source "../include/conf/usePCRE2.sh"
fi
if $usePCRE2; then
    pcre=-lpcre2-8
    echo Using $pcre
fi' >> build.tmp
echo -n "g++ -std=c++11 -o ../../faragebot engine/faragebot.cpp shared/libini.cpp -I \"${sleepypath}/include\" -I \"${sleepypath}/deps\" -I \"${sleepypath}/deps/include\" -I \"${sleepypath}/include/sleepy_discord/IncludeNonexistent\" -I \"../include\" -I \"../include/shared\" -L\"${sleepypath}/buildtools\" -L\"${sleepypath}/deps/cpr\" ${sleepypath}/deps/cpr/cpr/*.o -L\"${curDir}/src/bin\" -L/usr/bin -pthread -lcurl -lssl -lcrypto -lsleepy_discord -lfarage " >> build.tmp
echo '$pcre -ldl "$@"
if [[ $? -ne 0 ]] && [[ "$engversion" != "$oldengversion" ]]; then
    echo "#define FARAGE_ENGINE           \"$oldengversion\"" > ../include/conf/.farageEngineBuild.h
    exit 1
else
    echo Successfully compiled FarageBOT v$engversion
fi' >> build.tmp
mv build.tmp src/buildengine.sh
chmod +x src/buildengine.sh

cd $sleepypath
if $nogit; then
    echo "git is not installed! Unable to ensure SleepyDiscord is on develop branch!"
    echo "... Continuing anyway (against my better judgement)"
else
    git checkout develop
fi
echo ""
if ! $setup; then
    echo -n "Have you ran the SleepyDiscord setup script yet? [Y/n] "
    read ans
    if [[ $ans != y* ]] && [[ $ans != Y* ]]; then
        setup=true
    fi
fi
if $setup; then
    if $nogit; then
        echo "Cannot continue without git installed!"
        exit 1
    fi
    echo ""
    echo ""
    echo ""
    echo ""
    echo ""
    echo "    +~~~~~~~~~~~~~~~~~~!!!!!READ THIS!!!!!~~~~~~~~~~~~~~~~~~+"
    echo "    |                                                       |"
    echo "    |       Websocket++ will be installed seperately.       |"
    echo "    |  Do not select a Websocket library in the next step!  |"
    echo "    |                                                       |"
    echo "    +~~~~~~~~~~~~~~~~~~!!!!!READ THIS!!!!!~~~~~~~~~~~~~~~~~~+"
    echo ""
    python3 setup.py
    cd deps
    git clone https://github.com/zaphoyd/websocketpp.git
    mv websocketpp/websocketpp include/.
    rm -rf websocketpp
    rm -rf include/asio &> /dev/null
    rm include/asio.hpp &> /dev/null
    git clone https://github.com/chriskohlhoff/asio.git
    cd asio
    git checkout 22afb86
    mv asio/include/asio ../include/.
    mv asio/include/asio.hpp ../include/.
    cd ..
    rm -rf asio
#    cd cpr/cpr
#    g++ -I "../../include" -c error.cpp
#    if [[ $? != 0 ]]; then
#        $download https://raw.githubusercontent.com/whoshuu/cpr/master/cpr/error.cpp
#    fi
fi
cd "${sleepypath}/buildtools"
line=$(grep -n '^libcpr-patch:$' Makefile.linux)
if [[ ${#line} -gt 5 ]]; then
    echo "Patching Makefile.linux . . ."
    line=${line%%:*}
    let "endline=line+2"
    sed -i "s/libcpr-patch \?//; ${line},${endline}d" Makefile.linux
fi
make -f Makefile.linux
if [[ $? != 0 ]]; then
    cd $curDir
    echo "Error: Could not build SleepyDiscord."
    exit 1
fi
cd $curDir

if [ ! -d "src/bin" ]; then
    mkdir "src/bin"
fi

echo ""
echo -n "Would you like to use the PCRE2 regex engine instead of std::regex? [Y/n] "
read ans
if [[ $ans == y* ]] || [[ $ans == Y* ]]; then
    echo -n "Do you already have PCRE2 installed? [Y/n] "
    read ans
    if [[ $ans == y* ]] || [[ $ans == Y* ]]; then
        echo ""
        echo "If you did not configure PCRE2 to use jit compiling, you need to run the configure script for it again with '--enable-jit' set."
        echo ""
    else
        if [[ $download != false ]]; then
            pcre2ver=$($download - "https://www.pcre.org/" | grep -A 8 '<h2>Versions</h2>' | grep -m 1 -o -E 'version <b>[0-9.]+</b>' | sed 's#version <b>\([0-9.]\+\)</b>#\1#')
            echo ""
            echo "Downloading most recent pcre2 version $pcre2ver"
            echo ""
            $download "pcre2-${pcre2ver}.tar.gz" "https://ftp.pcre.org/pub/pcre/pcre2-${pcre2ver}.tar.gz"
            tar xzf "pcre2-${pcre2ver}.tar.gz"
            cd "pcre2-${pcre2ver}"
            ./configure --enable-jit
            make
            if [[ $? != 0 ]]; then
                cd ..
                echo "Error: Could not compile PCRE2."
                exit 1
            fi
            make install
            if [[ $? != 0 ]]; then
                echo ""
                echo "Error: PCRE2 installation failed. This is likely due to lack of permissions."
                echo -n "Would you like to try again with sudo? [Y/n] "
                read ans
                if [[ $ans == y* ]] || [[ $ans == Y* ]]; then
                    sudo make install
                else
                    cd ..
                    echo "You will need to manually install PCRE2 before continuing."
                    exit 1
                fi
            fi
            cd ..
        else
            echo ""
            echo "Skipping download, you will need to manually download and install pcre2 from: https://ftp.pcre.org/pub/pcre/"
            echo "When running the configure script, be sure to pass the '--enable-jit' switch, or it will not be compatible!"
        fi
    fi
    echo "usePCRE2=true" > include/conf/usePCRE2.sh
    echo ""
    echo "Building PCRE2 wrapper . . ."
    g++ -std=c++11 -o src/bin/pcre2_halfwrap.o -c src/shared/pcre2_halfwrap.cpp -I "include/shared" -fPIC
    if [[ $? != 0 ]]; then
        echo "Error: Could not build PCRE2 wrapper. Make sure PCRE2 is installed correctly."
        exit 1
    fi
else
    echo "usePCRE2=false" > include/conf/usePCRE2.sh
fi
echo ""
echo "FarageBOT has been successfully configured!"
echo ""
echo -n "Would you like to build the FarageAPI now? [Y/n] "
read ans
if [[ $ans == y* ]] || [[ $ans == Y* ]]; then
    cd src
    ./buildapi.sh
    if [[ $? != 0 ]]; then
        cd ..
        echo "Error: An error occurred while building the API. Try reinstalling?"
        exit 1
    fi
    cd ..
    echo ""
    echo -n "Would you like to build the Farage Engine now? [Y/n] "
    read ans
    if [[ $ans == y* ]] || [[ $ans == Y* ]]; then
        cd src
        ./buildengine.sh
        if [[ $? != 0 ]]; then
            cd ..
            echo "Error: An error occurred while building the Engine. Try reinstalling?"
            exit 1
        fi
        cd ..
    fi
fi

