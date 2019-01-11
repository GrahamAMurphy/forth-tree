#!/bin/sh
cat <<EOF >tmp$$
Here is the latest version of FRISC Forth for the
MPE power board with an RTX2010.  The object code
is in Motorola EXORmacs format.

- John Hayes

------------------------------cut here------------------------------------
EOF
pupmotor newforth | cat tmp$$ - | mail -s "FRISC Forth Distribution" mpeusers
rm tmp$$
