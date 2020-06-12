RED='\033[0;31m'
GREEN='\033[0;32m'
ORANGE='\033[0;33m'
NC='\033[0m' # No Color

trap ctrl_c INT

function ctrl_c() {
    printf "\n${ORANGE}[Runner]${NC} Exiting\n"
    kill $PID
    kill $PPID
    rm $EXEC 2>/dev/null
    rm $BUILD 2>/dev/null
    exit 1
}

EXEC="main"
SPATH="src"
BPATH="build"
BUILD="build_info.txt"

printf "${ORANGE}[Runner]${NC} Initialising\n"

mkdir $BPATH
cd $BPATH
rm -rf *
cmake ../$SPATH -DCMAKE_BUILD_TYPE=Debug #  -DCMAKE_EXPORT_COMPILE_COMMANDS=YES #  -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
#mv compile_commands.json ..
cd ..

printf "${ORANGE}[Runner]${NC} Building $EXEC\n"
cd $BPATH
rm $EXEC 2>/dev/null
rm $BUILD 2>/dev/null

cmake --build . --parallel 1 >$BUILD

if [ -f "$EXEC" ]; then
    printf "${ORANGE}[Runner]${NC} ${GREEN}Build succeded\n"
    cat $BUILD
    rm $BUILD 2>/dev/null
    cd ..
    printf "${ORANGE}[Runner]${NC} ${GREEN}Done $EXEC\n"
else
    printf "${ORANGE}[Runner]${NC} ${RED}$EXEC not found, build failed, errors:${NC}\n"
    cat $BUILD
    rm $BUILD 2>/dev/null
    cd ..
    printf "${ORANGE}[Runner]${NC} ${RED}-- end --${NC}\n"
fi
