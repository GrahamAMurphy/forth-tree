#!/bin/sh
# Count reused and new lines, both "all" and "non-comment" (NC)

echo "system reused new reused(NC) new(NC)"
for sys in crispboot
do
	cat `cat $sys-lib` | wc -l >tmp$$
	liblines=`cat tmp$$`
	cat `cat $sys-app` | wc -l >tmp$$
	applines=`cat tmp$$`
	squish `cat $sys-lib` | wc -l >tmp$$
	libsquish=`cat tmp$$`
	squish `cat $sys-app` | wc -l >tmp$$
	appsquish=`cat tmp$$`
	echo $sys $liblines $applines $libsquish $appsquish
done
rm tmp$$
