#!/bin/sh -
echo '/* This file is generated automatically: do not edit directly */'
echo '/*'
echo ' * Analyses'
echo ' */'
echo '#ifndef TABLES_ONLY'
for xx in $2; do
	echo "#define AN_$xx"
done
echo '#endif'
echo ''
echo '/*'
echo ' * Devices'
echo ' */'
for xx in $1; do
	echo "#define DEV_$xx"
done;
echo ''
echo '#define DEVICES_USED' \"$1\"
echo '#define ANALYSES_USED' \"$2\"
echo ''
echo "char Spice_Version[ ] = \"$3\";"
echo "char Spice_Notice[ ] = \"$4\";"
echo "char Spice_Build_Date[ ] = \"`date`\";";
echo ''
exit 0
