#!/bin/sh
(for trace in freja topex traps oops
do
	echo -n "$trace-i:1024:2:4:"
	sasubcache -v -s 1024 -a 2 -b 4 -c 100000 -l 1000000 ${trace}trace
	for w in m n
	do
		echo -n "$trace-m$w:1024:2:4:"
		sasubcache -v -m$w -s 1024 -a 2 -b 4 -c 100000 -l 1000000 \
					${trace}trace
	done
done) | tr ':' '\011' >sasub1024-2-4results
