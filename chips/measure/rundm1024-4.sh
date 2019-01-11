#!/bin/sh
(for trace in freja topex traps oops
do
	echo -n "$trace-i:1024:4:"
	dmcache -v -b 4 -c 100000 -l 1000000 -s 1024 ${trace}trace
	for w in a m n
	do
		echo -n "$trace-m$w:1024:4:"
		dmcache -v -b 4 -m$w -c 100000 -l 1000000 -s 1024 \
				${trace}trace
	done
done) | tr ':' '\011' >dm1024-4results
