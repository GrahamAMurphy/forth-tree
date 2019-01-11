#!/bin/sh
# Count lines written for reuse

near="msi11-lib nismag5-lib xgrs7-lib"
contour="cfi-lib crisp-lib"
mess="mdis7-lib epps4-lib grs3-lib mag8-lib mascs22-lib ns4-lib xrs6-lib"
crism="crism3-lib"
nh="lorri3-lib pepssi-lib"

wr=`sort $near | uniq | fgrep /project/neardpu/`
echo -n "NEAR"; cat $wr | wc -l
wr=`sort $contour | uniq | fgrep /project/contour/`
echo -n "CONTOUR"; cat $wr | wc -l
wr=`sort $mess | uniq | fgrep /project/messenger/instruments/`
echo -n "MESS"; cat $wr | wc -l
wr=`sort $crism | uniq | fgrep /project/crism/`
echo -n "CRISM"; cat $wr | wc -l
wr=`sort $nh | uniq | fgrep /project/pluto/instruments/flight/`
echo -n "NH"; cat $wr | wc -l
