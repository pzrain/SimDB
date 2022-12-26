FLAG_1=$1
IN_FILE=$2
OUT_FILE=$3
FLAG_4=$4

if ([ $# -eq 1 ] && [ $FLAG_1 = '-c' ]) || ([ $# -eq 4 ] && [ $FLAG_4 = '-c' ]); then
    if [[ ! -d build ]]; then
        echo "good"
    else
        rm -rf build
    fi
    mkdir -p build
    cd build
    cmake ..
    make -j8
    cd ..

    echo "Done!"
fi

mkdir -p database

if [ $# -ge 1 ] && [ $FLAG_1 = '-batch' ]; then
    echo "batch mode"
    ./bin/SimDB 2>msg.debug 1>$OUT_FILE $IN_FILE
else
    echo "cli mode"
    ./bin/SimDB 2>/dev/null
fi
