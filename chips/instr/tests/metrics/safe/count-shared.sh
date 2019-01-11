#!/bin/sh

echo -n $1" "
shift 1

sort $* >tmp$$
cat `cat tmp$$` | wc -l >tmp2$$
alllines=`cat tmp2$$`
cat `uniq tmp$$` | wc -l >tmp2$$
actlines=`cat tmp2$$`
echo $actlines $alllines
rm tmp$$ tmp2$$
