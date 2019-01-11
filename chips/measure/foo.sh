#!/bin/sh
(echo -n "freja-i:1024:4:"
dmsubcache -v -b 4 -c 100000 -l 1000000 -s 1024 frejatrace
for w in a m n
do
	echo -n "frejatrace-m$w:1024:4:"
	dmsubcache -v -b 4 -m$w -c 100000 -l 1000000 -s 1024 frejatrace
done) | tr ':' '\011'
