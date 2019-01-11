#!/bin/sh

near="msi11-app msi11-lib nismag5-app nismag5-lib xgrs7-app xgrs7-lib"
contour="cfi-app cfi-lib crisp-app crisp-lib"
mess="mdis-app mdis-lib epps4-app epps4-lib grs3-app grs3-lib \
	mag8-app mag8-lib ns4-app ns4-lib xrs6-app xrs6-lib"
mro="crism-app crism-lib"
nh="lorri-app lorri-lib pepssi-app pepssi-lib"		# TBD: early PEPSSI

instr="$contour $mess $mro $nh"
all="$near $instr"

messboot="bootroms/messdpuboot-app bootroms/messdpuboot-lib \
	bootroms/messepuboot-app bootroms/messepuboot-lib"
mroboot="bootroms/mroboot-app bootroms/mroboot-lib"
nhboot="bootroms/nhboot-app bootroms/nhboot-lib"

# Count lines with each family
count-shared.sh "NEAR" $near
count-shared.sh "CONTOUR" $contour
count-shared.sh "MESSENGER" $mess
count-shared.sh "NH" $nh

# Count lines across family lines
count-shared.sh "Instr" $contour $mess $mro $nh
count-shared.sh "All" $near $contour $mess $mro $nh

# Count lines including boot
count-shared.sh "MESS+boot" $mess $messboot
count-shared.sh "NH+boot" $nh $nhboot
count-shared.sh "Instr+boot" $contour $mess $messboot $mro $mroboot $nh $nhboot
count-shared.sh "All+boot" $near $contour $mess $messboot $mro $mroboot $nh $nhboot
