#!/bin/bash

if [ "$#" -gt "2" ] || [ "$#" -lt "1" ]; then
	echo "Syntax: ./set-installation-path.sh <installation-path> [--create-dir]"
else
	if [ -d "$1" ]; then
		touch .installdir
		echo $1 > .installdir
		echo "./set-installation-path.sh: Installation path set to $1" 
	else
		if [ "$2" != "--create-dir" ]; then
			echo "./set-installation-path.sh: Directory $1 does not exist. Exiting."
		else
			echo "./set-installation-path.sh: Directory $1 does not exist but will be created."
			mkdir -p $1
			touch .installdir
			echo $1 > .installdir
		fi
	fi
fi
