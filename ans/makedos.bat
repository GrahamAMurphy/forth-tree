gcc -O -Dcmach  -target sun4 -o gendefs gendefs.c 
go32 gendefs >defs.h
go32 meta/forth2m -o tdihc -w -i /usr/local/bin/forth -p cmach/macro.fr @files.dos >jhforthc.c
gcc -O -c jhforthc.c
rm -f jhforthc.c
gcc -O -Dcmach -target sun4 -c  filewrap.c
gcc -O -Dcmach -target sun4 -c  main.c
gcc -o forth jhforthc.o filewrap.o main.o
