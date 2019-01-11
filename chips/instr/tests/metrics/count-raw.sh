#!/bin/sh
# Count reused and new lines, both "all" and "non-comment" (NC)

echo "system reused new reused(NC) new(NC)"
for sys in msi11 nismag5 xgrs7 mimi hena \
		cfi crisp \
		mdis7 epps4 grs3 mag8 mascs22 ns4 xrs6 crism3 lorri3 pepssi \
		messepuboot mroboot nhboot
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
