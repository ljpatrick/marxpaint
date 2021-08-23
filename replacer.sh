#!/bin/bash

find . -type f -name '*tux*' | while read FILE ; do
    newfile="$(echo ${FILE} |sed -e 's/tux/marx/')" ;
    echo ${FILE} - ${newfile} ;
    mv "${FILE}" "${newfile}" ;
done 

