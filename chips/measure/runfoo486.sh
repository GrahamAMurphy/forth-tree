#!/bin/sh
for assoc in 1 2 4 8 16
do
	for trace in freja topex traps oops
	do
		(echo -n "$trace:"
		isa486cache -v -m -a $assoc -c 100000 -l 1000000 -s 1024 \
							${trace}trace
		) | tr ':' '\011'
	done
done
