#!/bin/bash

cd ../../ # Cinnamon root folder

# Make sure we cd'd to a good directory
currentDirectory=$(echo "${PWD##*/}")
if [[ "$currentDirectory" != "Cinnamon" ]]
then
	echo "Executing in wrong directory, aborting"
	exit 1
fi

declare -i nDirectoriesRemoved=0
directories=( "bin-int" "bin" )

for i in "${directories[@]}"
do
	if [[ -d "$i" ]]
	then
		echo "removing $i"
		rm -rf "$i"

		nDirectoriesRemoved+=1
	fi
done

echo "directories removed: $nDirectoriesRemoved"
