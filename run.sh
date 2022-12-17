COMPILE_FLAG=$1

if [ $# -eq 1 ] && [ $COMPILE_FLAG = '-c' ]; then
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
./bin/SimDB 2>msg.debug
# rm -r database
