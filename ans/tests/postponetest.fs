\ checks that postpone works correctly with words with special
\ compilation semantics

\ by M. Anton Ertl 1996

\ This file is based on John Hayes' core.fr (coretest.fs), which has
\ the following copyright notice:

\ (C) 1995 JOHNS HOPKINS UNIVERSITY / APPLIED PHYSICS LABORATORY
\ MAY BE DISTRIBUTED FREELY AS LONG AS THIS COPYRIGHT NOTICE REMAINS.

\ my contributions to this file are in the public domain

\ you have to load John Hayes' tester.fs (=tester.fr) and coretest.fs
\ (core.fr) first

\ These tests are especially useful for showing that state-smart
\ implementations of words with special compilation semantics,
\ combined with a straight-forward implementation of POSTPONE (and
\ [COMPILE]) do not conform to the ANS Forth standard. The essential
\ sentences in the standad are:

\ 6.1.2033 POSTPONE CORE
\ ...
\ Compilation: ( <spaces>name -- ) 

\ Skip leading space delimiters. Parse name delimited by a space. Find
\ name. Append the compilation semantics of name to the current
\ definition.

\ 6.2.2530 [COMPILE] bracket-compile CORE EXT 
\ ...
\ Compilation: ( <spaces>name -- ) 

\ Skip leading space delimiters. Parse name delimited by a space. Find
\ name. If name has other than default compilation semantics, append
\ them to the current definition;...


\ Note that the compilation semantics are appended, not some
\ state-dependent semantics.

\ first I test against a non-ANS solution suggested by Bernd Paysan

: state@-now ( -- f )
    state @ ; immediate

: state@ ( -- f )
    postpone state@-now ;

{ state@ -> state @ }

\ here i test postpone with all core words with special compilation
\ semantics.

testing postpone (

: postpone-(
    postpone ( ;

{ : pp1 [ postpone-( does nothing ) ] ; -> }
{ here pp1 -> here }

testing postpone +loop

: postpone-+loop
    postpone +loop ;

{ : pgd2 do i -1 [ postpone-+loop ] ; -> }
{ 1 4 pgd2 -> 4 3 2 1 }
{ -1 2 pgd2 -> 2 1 0 -1 }
{ mid-uint mid-uint+1 pgd2 -> mid-uint+1 mid-uint }

{ : pgd4 do 1 0 do j loop -1 [ postpone-+loop ] ; -> }
{ 1 4 pgd4 -> 4 3 2 1 }
{ -1 2 pgd4 -> 2 1 0 -1 }
{ mid-uint mid-uint+1 pgd4 -> mid-uint+1 mid-uint }

testing postpone ."

: postpone-."
    postpone ." ;

: pdq2 [ postpone-." you should see this later. " ] cr ;
: pdq1 [ postpone-." you should see this first. " ] cr ;
{ pdq1 pdq2 -> }

testing postpone ;
: postpone-;
    postpone ; ;

{ : psc [ postpone-; -> }
{ psc -> }    

testing postpone abort"

: postpone-abort"
    postpone abort" ;

{ : paq1 [ postpone-abort" this should not abort" ] ; -> }

testing postpone begin
: postpone-begin
    postpone begin ;

{ : pb3 [ postpone-begin ] dup 5 < while dup 1+ repeat ; -> }
{ 0 pb3 -> 0 1 2 3 4 5 }
{ 4 pb3 -> 4 5 }
{ 5 pb3 -> 5 }
{ 6 pb3 -> 6 }

{ : pb4 [ postpone-begin ] dup 1+ dup 5 > until ; -> }
{ 3 pb4 -> 3 4 5 6 }
{ 5 pb4 -> 5 6 }
{ 6 pb4 -> 6 7 }

{ : pb5 [ postpone-begin ] dup 2 > while dup 5 < while dup 1+ repeat 123 else 345 then ; -> }
{ 1 pb5 -> 1 345 }
{ 2 pb5 -> 2 345 }
{ 3 pb5 -> 3 4 5 123 }
{ 4 pb5 -> 4 5 123 }
{ 5 pb5 -> 5 123 }

testing postpone do
: postpone-do
    postpone do ;

{ : pdo1 [ postpone-do ] i loop ; -> }
{ 4 1 pdo1 -> 1 2 3 }
{ 2 -1 pdo1 -> -1 0 1 }
{ mid-uint+1 mid-uint pdo1 -> mid-uint }

{ : pdo2 [ postpone-do ] i -1 +loop ; -> }
{ 1 4 pdo2 -> 4 3 2 1 }
{ -1 2 pdo2 -> 2 1 0 -1 }
{ mid-uint mid-uint+1 pdo2 -> mid-uint+1 mid-uint }

{ : pdo3 [ postpone-do ] 1 0 [ postpone-do ] j loop loop ; -> }
{ 4 1 pdo3 -> 1 2 3 }
{ 2 -1 pdo3 -> -1 0 1 }
{ mid-uint+1 mid-uint pdo3 -> mid-uint }

{ : pdo4 [ postpone-do ] 1 0 [ postpone-do ] j loop -1 +loop ; -> }
{ 1 4 pdo4 -> 4 3 2 1 }
{ -1 2 pdo4 -> 2 1 0 -1 }
{ mid-uint mid-uint+1 pdo4 -> mid-uint+1 mid-uint }

{ : pdo5 123 swap 0 [ postpone-do ] i 4 > if drop 234 leave then loop ; -> }
{ 1 pdo5 -> 123 }
{ 5 pdo5 -> 123 }
{ 6 pdo5 -> 234 }

{ : pdo6  ( pat: {0 0},{0 0}{1 0}{1 1},{0 0}{1 0}{1 1}{2 0}{2 1}{2 2} )
   0 swap 0 [ postpone-do ]
      i 1+ 0 [ postpone-do ] i j + 3 = if i unloop i unloop exit then 1+ loop
    loop ; -> }
{ 1 pdo6 -> 1 }
{ 2 pdo6 -> 3 }
{ 3 pdo6 -> 4 1 2 }

testing postpone does>
: postpone-does>
    postpone does> ;

{ : pdoes1 [ postpone-does> ] @ 1 + ; -> }
{ : pdoes2 [ postpone-does> ] @ 2 + ; -> }
{ create pcr1 -> }
{ pcr1 -> here }
{ ' pcr1 >body -> here }
{ 1 , -> }
{ pcr1 @ -> 1 }
{ pdoes1 -> }
{ pcr1 -> 2 }
{ pdoes2 -> }
{ pcr1 -> 3 }

{ : pweird: create [ postpone-does> ] 1 + [ postpone-does> ] 2 + ; -> }
{ pweird: pw1 -> }
{ ' pw1 >body -> here }
{ pw1 -> here 1 + }
{ pw1 -> here 2 + }

testing postpone else
: postpone-else
    postpone else ;

{ : pelse1 if 123 [ postpone-else ] 234 then ; -> }
{ 0 pelse1 -> 234 }
{ 1 pelse1 -> 123 }

{ : pelse2 begin dup 2 > while dup 5 < while dup 1+ repeat 123 [ postpone-else ] 345 then ; -> }
{ 1 pelse2 -> 1 345 }
{ 2 pelse2 -> 2 345 }
{ 3 pelse2 -> 3 4 5 123 }
{ 4 pelse2 -> 4 5 123 }
{ 5 pelse2 -> 5 123 }

testing postpone if
: postpone-if
    postpone if ;

{ : pif1 [ postpone-if ] 123 then ; -> }
{ : pif2 [ postpone-if ] 123 else 234 then ; -> }
{ 0 pif1 -> }
{ 1 pif1 -> 123 }
{ -1 pif1 -> 123 }
{ 0 pif2 -> 234 }
{ 1 pif2 -> 123 }
{ -1 pif1 -> 123 }

{ : pif6 ( n -- 0,1,..n ) dup [ postpone-if ] dup >r 1- recurse r> then ; -> }
{ 0 pif6 -> 0 }
{ 1 pif6 -> 0 1 }
{ 2 pif6 -> 0 1 2 }
{ 3 pif6 -> 0 1 2 3 }
{ 4 pif6 -> 0 1 2 3 4 }

testing postpone literal
: postpone-literal
    postpone literal ;

{ : plit [ 42 postpone-literal ] ; -> }
{ plit -> 42 }

testing postpone loop
: postpone-loop
    postpone loop ;

{ : ploop1 do i [ postpone-loop ] ; -> }
{ 4 1 ploop1 -> 1 2 3 }
{ 2 -1 ploop1 -> -1 0 1 }
{ mid-uint+1 mid-uint ploop1 -> mid-uint }

{ : ploop3 do 1 0 do j [ postpone-loop ] [ postpone-loop ] ; -> }
{ 4 1 ploop3 -> 1 2 3 }
{ 2 -1 ploop3 -> -1 0 1 }
{ mid-uint+1 mid-uint ploop3 -> mid-uint }

{ : ploop4 do 1 0 do j [ postpone-loop ] -1 +loop ; -> }
{ 1 4 ploop4 -> 4 3 2 1 }
{ -1 2 ploop4 -> 2 1 0 -1 }
{ mid-uint mid-uint+1 ploop4 -> mid-uint+1 mid-uint }

{ : ploop5 123 swap 0 do i 4 > if drop 234 leave then [ postpone-loop ] ; -> }
{ 1 ploop5 -> 123 }
{ 5 ploop5 -> 123 }
{ 6 ploop5 -> 234 }

{ : ploop6  ( pat: {0 0},{0 0}{1 0}{1 1},{0 0}{1 0}{1 1}{2 0}{2 1}{2 2} )
   0 swap 0 do
      i 1+ 0 do i j + 3 = if i unloop i unloop exit then 1+ [ postpone-loop ]
    [ postpone-loop ] ; -> }
{ 1 ploop6 -> 1 }
{ 2 ploop6 -> 3 }
{ 3 ploop6 -> 4 1 2 }

testing postpone postpone
: postpone-postpone
    postpone postpone ;

{ : ppp1 123 ; -> }
{ : ppp4 [ postpone-postpone ppp1 ] ; immediate -> }
{ : ppp5 ppp4 ; -> }
{ ppp5 -> 123 }
{ : ppp6 345 ; immediate -> }
{ : ppp7 [ postpone-postpone ppp6 ] ; -> }
{ ppp7 -> 345 }

testing postpone recurse
: postpone-recurse
    postpone recurse ;

{ : grec ( n -- 0,1,..n ) dup if dup >r 1- [ postpone-recurse ] r> then ; -> }
{ 0 grec -> 0 }
{ 1 grec -> 0 1 }
{ 2 grec -> 0 1 2 }
{ 3 grec -> 0 1 2 3 }
{ 4 grec -> 0 1 2 3 4 }

testing postpone repeat
: postpone-repeat
    postpone repeat ;

{ : prep3 begin dup 5 < while dup 1+ [ postpone-repeat ] ; -> }
{ 0 prep3 -> 0 1 2 3 4 5 }
{ 4 prep3 -> 4 5 }
{ 5 prep3 -> 5 }
{ 6 prep3 -> 6 }

{ : prep5 begin dup 2 > while dup 5 < while dup 1+ [ postpone-repeat ] 123 else 345 then ; -> }
{ 1 prep5 -> 1 345 }
{ 2 prep5 -> 2 345 }
{ 3 prep5 -> 3 4 5 123 }
{ 4 prep5 -> 4 5 123 }
{ 5 prep5 -> 5 123 }

testing postpone s"
: postpone-s"
    postpone s" ;

{ : psq4 [ postpone-s" xy" ] ; -> }
{ psq4 swap drop -> 2 }
{ psq4 drop dup c@ swap char+ c@ -> 58 59 }

testing postpone then
: postpone-then
    postpone then ;

{ : pth1 if 123 [ postpone-then ] ; -> }
{ : pth2 if 123 else 234 [ postpone-then ] ; -> }
{ 0 pth1 -> }
{ 1 pth1 -> 123 }
{ -1 pth1 -> 123 }
{ 0 pth2 -> 234 }
{ 1 pth2 -> 123 }
{ -1 pth1 -> 123 }

{ : pth5 begin dup 2 > while dup 5 < while dup 1+ repeat 123 else 345 [ postpone-then ] ; -> }
{ 1 pth5 -> 1 345 }
{ 2 pth5 -> 2 345 }
{ 3 pth5 -> 3 4 5 123 }
{ 4 pth5 -> 4 5 123 }
{ 5 pth5 -> 5 123 }

{ : pth6 ( n -- 0,1,..n ) dup if dup >r 1- recurse r> [ postpone-then ] ; -> }
{ 0 pth6 -> 0 }
{ 1 pth6 -> 0 1 }
{ 2 pth6 -> 0 1 2 }
{ 3 pth6 -> 0 1 2 3 }
{ 4 pth6 -> 0 1 2 3 4 }

testing postpone until
: postpone-until
    postpone until ;

{ : punt4 begin dup 1+ dup 5 > [ postpone-until ] ; -> }
{ 3 punt4 -> 3 4 5 6 }
{ 5 punt4 -> 5 6 }
{ 6 punt4 -> 6 7 }

testing postpone while
: postpone-while
    postpone while ;

{ : pwh3 begin dup 5 < [ postpone-while ] dup 1+ repeat ; -> }
{ 0 pwh3 -> 0 1 2 3 4 5 }
{ 4 pwh3 -> 4 5 }
{ 5 pwh3 -> 5 }
{ 6 pwh3 -> 6 }

{ : pwh5 begin dup 2 > [ postpone-while ] dup 5 < [ postpone-while ] dup 1+ repeat 123 else 345 then ; -> }
{ 1 pwh5 -> 1 345 }
{ 2 pwh5 -> 2 345 }
{ 3 pwh5 -> 3 4 5 123 }
{ 4 pwh5 -> 4 5 123 }
{ 5 pwh5 -> 5 123 }


testing postpone [
: postpone-[
    postpone [ ;

{ here postpone-[ -> here }

testing postpone [']
: postpone-[']
    postpone ['] ;

{ : ptick1 123 ; -> }
{ : ptick2 [ postpone-['] ptick1 ] ; immediate -> }
{ ptick2 execute -> 123 }

testing postpone [char]
: postpone-[char]
    postpone [char] ;

{ : pchar1 [ postpone-[char] x ] ; -> }
{ : pchar2 [ postpone-[char] hello ] ; -> }
{ pchar1 -> 78 }
{ pchar2 -> 68 }

