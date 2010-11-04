#!/bin/bash

a=0
b=0
for map in $(ls maps); do
    res=$(java -jar tools/PlayGame.jar maps/$map 1000 1000 log.txt $1 "./MyBot" 2>&1 > /dev/null | tail -1 | awk '{print $2}')
    echo $res
    if [ $res -eq "1" ]; then
	let a++
    else
	let b++
    fi
done

echo
echo "Old bot: "$a
echo "New bot: "$b