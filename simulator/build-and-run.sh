#! /bin/bash

set -e
set -u

EXE_NAME=sim

DN_THIS=`dirname $BASH_SOURCE`
DN_BUILD=$DN_THIS/target

printf "Building ...\n"
mkdir -p $DN_BUILD
pushd $DN_BUILD > /dev/null
cmake ..
#cmake -DCMAKE_BUILD_TYPE=Debug ..
#time make -j
time make
popd > /dev/null

printf "\n"
printf "Running $EXE_NAME ...\n"
#time valgrind target/$EXE_NAME "$@"
time target/$EXE_NAME "$@"
