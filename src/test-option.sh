#!/bin/sh

echo 'main(){}' > dummy.c
gcc $1 dummy.c > /dev/null 2>&1 && echo $1
rm -f dummy.c a.out

