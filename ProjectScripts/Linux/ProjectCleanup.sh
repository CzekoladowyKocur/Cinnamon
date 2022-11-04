#!/bin/bash

cd ../../ # Cinnamon root folder

# Make sure we cd'd to a good directory
currentDirectory=$(echo "${PWD##*/}")
if [[ "$currentDirectory" != "Cinnamon" ]]
then
	echo "Executing in wrong directory, aborting"
	exit 1
fi

if [[ -d "bin" ]]
then
	rm -rf bin
fi

if [[ -d "bin-int" ]]
then
	rm -rf bin-int
fi

if [[ -d ".vscode" ]]
then
	rm -rf .vscode 
fi

if [[ -f "null.d" ]]
then
	rm null.d
fi

find . -name Makefile -type f -delete
