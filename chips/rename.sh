#!/bin/sh
for i in *.C
do
	mv $i `echo $i | tr '[A-Z]' '[a-z]'`
done
