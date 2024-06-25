exec_name=smtpd
target_exec_name=smtpd
could_build=0

if [ $# -ge 1 ]; then
    target_exec_name=$1
fi

cd ./src
make clean
CC=gcc make

if [ $? -eq 0 ]; then
    mv $exec_name ../$target_exec_name
    could_build=1
fi

make clean

if [ $could_build -eq 1 ]; then
    echo ""
    echo "********************"
    echo "* Build successful *"
    echo "********************"
    echo ""
fi

