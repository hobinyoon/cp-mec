#! /bin/bash

set -e
set -u

DN_THIS=`dirname $BASH_SOURCE`
DN_BUILD=$DN_THIS/target

printf "Building ...\n"
mkdir -p $DN_BUILD
pushd $DN_BUILD > /dev/null
cmake ..
#cmake -DCMAKE_BUILD_TYPE=Debug ..

if [ `hostname` == "u1604-16g" ]; then
  time make
else
  time make -j
fi

popd > /dev/null

printf "\n"
printf "Running calc-req-at-co ...\n"
#time valgrind target/calc-req-at-co "$@"
time target/calc-req-at-co "$@"
