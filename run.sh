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

mkdir -p database
./bin/SimDB 2>msg.err
