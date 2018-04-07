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

# Ignore the error. Seem to be common in the NFS without clock sync.
#   make: Warning: File `Makefile' has modification time 0.12 s in the future
#   http://stackoverflow.com/questions/13745645/makefile-clock-skew-detected

#time make -j
time make
popd > /dev/null

printf "\n"
printf "Clustering ...\n"
time target/cluster "$@"
