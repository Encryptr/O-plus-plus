#!/bin/bash

yourfilenames=`find ./tests/ -name "*.opp"`
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

for eachfile in $yourfilenames
do
	read line < $eachfile
	retval=$(echo $line | tr -dc '0-9')

	./o++ -w $@ $eachfile 
	gcc out.o
	./a.out
   got=$?

   if [ ! $got = "$retval" ] 
   then
   	echo -e "[ ${RED} FAILED ${NC} ] ${eachfile} got:" $got
   else
   	echo -e "[ ${GREEN} PASS ${NC} ] ${eachfile}"
   fi

   rm a.out
done