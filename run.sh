#! /bin/bash 
echo
echo "______________S T A R T   J O B_____________________"
echo
echo "PL/1  ----->  Assembler"
echo "continue? (Y/n)"
read ANSW
if [ "$ANSW" != "n" ] 
then
    echo "./build/pl1c -i examppl.pli -o ./build/examppl.ass"
    ./build/pl1c -i examppl.pli -o ./build/examppl.ass
else 
    exit 1
fi
echo
echo "Assembler  ----->  Object image"
echo "continue? (Y/n)"
read ANSW
if [ "$ANSW" != "n" ] 
then
    echo "./build/pl1l -i examppl.ass -o examppl.bin"
    cd build && ./pl1l -i examppl.ass -o examppl.bin
else 
    exit 1
fi
echo
echo "Load, run and debug Object image"
echo "continue? (Y/n)"
read ANSW
if [ "$ANSW" != "n" ] 
then
    echo ./ibm360vm ../spis.mod
    ./ibm360vm ../spis.mod
else 
    exit 1
fi

echo
echo "______________F I N I S H     J O B__________________"
