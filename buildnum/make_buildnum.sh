#!/bin/bash
# FILE: make_buildnum.sh

readonly MYDIR=$( dirname "$(readlink -f "$0")" )

(
cd "$MYDIR"

version=$(cat major_version | tr -d '\r\n')
old=$(cat build.number | tr -d '\r\n')
((++old))
combo="${version}.${old}"

echo "Build is ${version}${old}"

echo $old | bc > build.number
#versión..
#echo "$version`sed  's/^ *//' build.number` - `date`" > version.number
echo "$combo - `date`" > version.number
#header
echo "#ifndef BUILD_NUMBER_STR" > build_number.h
echo "#define BUILD_NUMBER_STR \"$combo\"" >> build_number.h
echo "#endif" >> build_number.h

echo "#ifndef VERSION_STR" >> build_number.h
echo "#define VERSION_STR \"$combo - `date`\"" >> build_number.h
echo "#endif" >> build_number.h

echo "#ifndef VERSION_STR_SHORT" >> build_number.h
echo "#define VERSION_STR_SHORT \"$combo\"" >> build_number.h
echo "#endif" >> build_number.h

)
