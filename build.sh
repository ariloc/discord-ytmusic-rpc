#! /bin/bash

echo "Checking dependencies... "
for name in cmake g++ python3.9
do
    if command -v $name >/dev/null 2>&1;
    then
        echo "$name OK."
    else
        echo "$name not installed! Please install it accordingly."
        exit 1
    fi
done
echo -e "All dependencies OK.\n"

echo "Entering source folder..."
cd ./discord-rpc-buttons

if [ -d "./build" ]
then
   echo "'build' folder exists. Cleaning all its contents..."
   rm -r build
fi

mkdir build

echo "Entering build folder..."
cd build

echo -e "\nSetting up CMake..."
cmake .. -DCMAKE_INSTALL_PREFIX="../../"

echo -e "\nBuilding discord-rpc-buttons library..."
cmake --build . --config Release --target install

echo -e "\nCleaning up 'build' folder..."
cd ..
rm -r build

echo -e "\nCompiling program using Makefile..."
cd ..
make

echo "Done!"
