#!/bin/bash

a=0
b=0
draw=0
for i in `seq $1 $2`; do
    res=$(java -jar tools/PlayGame.jar maps/map${i}.txt 1000 1000 log.txt $3 "./MyBot" 2>&1 > /dev/null | tail -1 | awk '{print $2}')
    if [ -z $res ]; then
	let draw++
    elif [ $res -eq "1" ]; then
	let a++
    else
	let b++
    fi
done

echo -n $a " " $b " " $draw