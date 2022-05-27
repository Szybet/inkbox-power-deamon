#!/bin/bash

# cd src/
#source /mnt/HDD/Project/qt-kobo/koxtoolchain/refs/x-compile.sh kobo env
# /mnt/HDD/Project/qt-kobo/x-tools/arm-kobo-linux-gnueabihf/bin/arm-kobo-linux-gnueabihf-g++ -o ../sleep_manager.bin main.cpp
cmake build

servername="root@192.168.0.147"
passwd="ZX12cw"

sshpass -p $passwd ssh $servername "bash -c \"ifsctl mnt rootfs rw\""
sshpass -p $passwd ssh $servername "bash -c \"rm /sleep_manager.bin\""
sshpass -p $passwd scp build/sleep_manager.bin $servername:/

sshpass -p $passwd ssh $servername "bash -c \"sync\""

# Normal launch
#sshpass -p $passwd ssh $servername "bash -c \"DEBUG=true /sleep_manager.bin\""

# For chroot
sshpass -p $passwd ssh $servername "bash -c \"rm /kobo/sleep_manager.bin\""
sshpass -p $passwd ssh $servername "bash -c \"mv /sleep_manager.bin /kobo/\""
sshpass -p $passwd ssh $servername "chroot /kobo sh -c \"DEBUG=true /sleep_manager.bin\""