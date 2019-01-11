hex

\ Johns Hopkins University / Applied Physics Laboratory
\ Eacker case structure.  This case structure is in common
\ use and is standardized in the CORE Extensions Word Set of
\ the ANS Forth standard.  (It is inferior to the case structure
\ provided in my Forth!)
\ Copied from Appendix A of ANS Forth standard.

0 constant case immediate

: of		\ ( #of -- orig #of+1 )
   1+ >r				\ keep count of 'of's
   postpone over postpone =
   postpone if postpone drop		\ compile code to test argument
   r>
; immediate

: endof		\ ( orig1 #of -- orig2 #of )
   >r postpone else r> ; immediate

: endcase	\ ( orig1..orign n -- )
   postpone drop			\ compile drop if no match
   ?dup if 0 do postpone then loop then	\ resolve all 'endof' branches
; immediate

0 constant EndOfCodeLight
1 constant PopLight
2 constant UpdateLight
4 constant UnwindLight
8 constant PushglobalLight
10 constant PushintLight
20 constant PushLight
40 constant MkapLight
80 constant CollectGarbageLight

\ lighttoturnon
: turnOnLight
    \ choose one or the other of the following
    \ .
    100002 w!
    \ drop
;

\ ( throwValue --- )
: throw
    cr
    s" THROWING a #" type
    .
    s" #" type
    cr
    begin
        1 drop
    again
    abort
;

decimal

\ The following are Node-type constants 
\ Nodes are 3 stack element structures: 
\ nodeType nodeLeft nodeRight 
1 constant NNum  \ heap elements that are just numbers
2 constant NAp   \ heap elements that are function applications
3 constant NGlobal \ heap nodes that point to supercombinator code defns
4 constant NInd  \ indirection nodes for lazy evaluation

\ The following are instruction constants 
0   constant EndOfCode
128 constant Unwind
129 constant Pushglobal
130 constant Pushint
131 constant Push
132 constant Mkap
133 constant UpdateI
134 constant Pop
135 constant Alloc
136 constant Slide
137 constant EvalI
138 constant Add
139 constant Sub
140 constant Mul
141 constant Div
142 constant Neg
143 constant Eq
144 constant Ne
145 constant Lt
146 constant Le
147 constant Gt
148 constant Ge
149 constant Cond
150 constant Purge

\ throw constants
0  constant noThrow
1  constant throwGlobalsFull
2  constant throwUnknownGlobal
10 constant throwHeapFull
20 constant throwAddrStackFull
21 constant throwAddrStackEmpty
22 constant throwAddrInvalidPeek
23 constant throwAddrInvalidOverwrite
30 constant throwDumpStackFull
31 constant throwDumpStackEmpty
32 constant throwDumpInvalidPeek
33 constant throwDumpInvalidOverwrite
40 constant throwInstrStackFull
41 constant throwInstrStackEmpty
100 constant throwUnboxIntProblem
101 constant throwUnboxBooleanProblem

\ Association List management

1024 constant GMGLOBALStotalsize
create GMGLOBALS GMGLOBALStotalsize allot
variable GMGLOBALSnextunused

\ initializeGlobals creates an initial association list for the globals
\ ( -- )
: initializeGlobals
    0 GMGLOBALSnextunused !
;

\ Returns the number of elements in the global list.
( -- numelements)
: aSize
   GMGLOBALSnextunused @ 8 / 
;

\ Updates a global to point to a new heap address.
\ ( heapAddr n -- )
: aUpdate
    false
    GMGLOBALS GMGLOBALSnextunused @ +
    GMGLOBALS
    2dup <> if
        do \ ?do
            drop  \ heapAddr globalnum
            dup   \ heapAddr globalnum globalnum 
            i @   \ heapAddr globalnum globalnum nextnum
            = if  \ heapAddr globalnum
                i   \ heapAddr globalnum I
                true   \ heapAddr globalnum I true
                leave
            then
            false   \ heapAddr globalnum false
        8 +loop
    else
        2drop
    then
    if \ heapAddr globalnum I
        nip \ heapAddr I 
        4 + ! \ ()
    else \ heapAddr globalnum
        throwUnknownGlobal throw
    then
;

\ aLookup returns the heap address of the specified global name
\ ( globalnum -- addr )
: aLookup
    false
    GMGLOBALS GMGLOBALSnextunused @ +
    GMGLOBALS
    2dup <> if
        do \ ?do
            drop  \ globalnum
            dup   \ globalnum globalnum 
            i @   \ globalnum globalnum nextnum
            = if  \ globalnum
                i   \ globalnum I
                true   \ globalnum I true
                leave
            then
            false   \ globalnum false
        8 +loop
    else
        2drop
    then
    if \ globalnum I
        4 + @ \ globalnum heapAddr
        nip \ heapAddr
    else \ globalnum
        throwUnknownGlobal throw
    then
;

\ aFetch is different than aLookup, because it fetches the nth map
\ element, as opposed to the heapAddress of a specific global number.
\ Used for garbage collection
\ ( n -- globalnum heapaddr )
: aFetch
    8 * \ ninbytes
    GMGLOBALS + \ fetchAddr
    dup  \ fetchAddr fetchAddr
    @    \ fetchAddr globalnum
    swap \ globalnum fetchAddr
    4 +  \ globalnum heapfetchAddr
    @    \ globalnum heapaddr
;

\ aInsert adds a globalnum -> heap address association to the GMGLOBALS list
\ ( heapAddr globalnum -- )
: aInsert
    GMGLOBALStotalsize 8 -
    GMGLOBALSnextunused @
    < if
        throwGlobalsFull throw
    then
    GMGLOBALS GMGLOBALSnextunused @ + ! \ heapAddr
    GMGLOBALS GMGLOBALSnextunused @ + 4 + ! \ ()
    GMGLOBALSnextunused @ 8 + GMGLOBALSnextunused !
;

\
\ Address stack management
\

1024 constant ADDRSTACKtotalsize
create ADDRSTACKbase ADDRSTACKtotalsize allot
variable ADDRSTACKcurrenttop
variable ADDRSTACKcurrentbase

: initializeAddrStack
    4 negate ADDRSTACKcurrenttop !
    ADDRSTACKbase ADDRSTACKcurrentbase !
;

\ addrStackGetBaseAndCurrent provides the current address stack
\ "context".  Useful in combination with the dump stack.
\ ( -- addrStackCurrentTop addrStackCurrentBase )
: addrStackGetBaseAndCurrent
    ADDRSTACKcurrenttop @
    ADDRSTACKcurrentbase @
;

\ addrStackPutBaseAndCurrent installs a new context for the address
\ stack.  Useful in combination with the dump stack.
\ ( addrStackCurrentTop addrStackCurrentBase -- )
: addrStackPutBaseAndCurrent
    ADDRSTACKcurrentbase !
    ADDRSTACKcurrenttop !
;

\ addrStackReInit essentially starts a new stack frame on top of the
\ current one.  This is done by just setting the the current top value
\ to the empty state.
\ ( -- )
: addrStackReInit
    ADDRSTACKcurrentbase @
    ADDRSTACKcurrenttop @ 4 +
    +
    ADDRSTACKcurrentbase !
    4 negate ADDRSTACKcurrenttop !
;

\ addrStackDepth inserts the depth of the address stack onto the stack
\ ( -- depth )
: addrStackDepth
    ADDRSTACKcurrenttop @ \ currenttop
    0< if
        0
    else
        ADDRSTACKcurrenttop @ \ currenttop
        4 / 1 +
    then
;

\ addrStackTotalDepth returns the total depth of the address stack, ignoring
\ ( -- totaldepth )
: addrStackTotalDepth
    addrStackDepth \ depth
    ADDRSTACKcurrentbase @ ADDRSTACKbase - 4 / \ depth basedepth
    + \ totaldepth
;

\ addrStackTotalPeek peeks into the actual address stack, regardless
\ of whether it's been reinitialized.
\ ( nth -- heapAddr )
: addrStackTotalPeek
    addrStackTotalDepth 1 -  \ nth depth
    4 *                   \ nth depthinbytes
    swap                  \ depthinbytes nth
    4 *                   \ depthinbytes nthinbytes
    -                     \ grabfromhere
    dup                   \ grabfromhere grabfromhere
    0< if
        throwAddrInvalidPeek throw
    then
    ADDRSTACKbase + @     \ heapAddr
;

\ addrStackTotalOverwrite writes into the actual address stack, regardless
\ of whether it's been reinitialized.
\ ( heapAddr nth -- )
: addrStackTotalOverwrite
    addrStackTotalDepth 1 -  \ heapAddr nth depth
    4 *                   \ heapAddr nth depthinbytes
    swap                  \ heapAddr depthinbytes nth
    4 *                   \ heapAddr depthinbytes nthinbytes
    -                     \ heapAddr grabfromhere
    dup                   \ heapAddr grabfromhere grabfromhere
    0< if
        throwAddrInvalidPeek throw
    then
    ADDRSTACKbase + !     \ heapAddr
;

\ addrStackPeek peeks at the nth stack element where n is on the stack
\ ( nth -- heapAddr )
: addrStackPeek
    ADDRSTACKcurrenttop @ \ nth currenttop
    swap                  \ currenttop nth
    4 *                   \ currenttop nth*4
    -                     \ grabfromhere
    dup                   \ grabfromhere grabfromhere
    0< if
        throwAddrInvalidPeek throw
    then
    ADDRSTACKcurrentbase @ + @            \ heapAddr
;

\ addrStackOverwrite modifies the nth stack element and replaces it with
\ heapAddr
\ ( heapAddr nth -- )
: addrStackOverwrite
    ADDRSTACKcurrenttop @ \ heapAddr nth currenttop
    swap                  \ heapAddr currenttop nth
    4 *                   \ heapAddr currenttop nth*4
    -                     \ heapAddr overwritepoint
    dup                   \ heapAddr overwritepoint overwritepoint
    0< if                 \ heapAddr overwritepoint
        throwAddrInvalidOverwrite throw
    then
    ADDRSTACKcurrentbase @ + !         \ ()
;

\ addrStackPush is designed to push a heap address onto the address stack
\ ( heapAddr -- )
: addrStackPush
    ADDRSTACKtotalsize 8 -
    ADDRSTACKcurrenttop @
    < if
        throwAddrStackFull throw
    then
    ADDRSTACKcurrenttop @ 4 + ADDRSTACKcurrenttop !
    ADDRSTACKcurrentbase @ ADDRSTACKcurrenttop @ + ! ( )
;

\ addrStackPop is designed to pop a heap address from the address stack
\ ( -- heapAddr )
: addrStackPop
    ADDRSTACKcurrenttop @
    0< if
        throwAddrStackEmpty throw
    else
        ADDRSTACKcurrentbase @ ADDRSTACKcurrenttop @ + @ \ heapAddr
        ADDRSTACKcurrenttop @ 4 - ADDRSTACKcurrenttop ! \ heapAddr
    then
;

\ addrStackSlide is designed to slide the address stack.
\ Note it needs to protect
\ against sliding the stack too far.
\ ( n -- )
: addrStackSlide
    ADDRSTACKcurrentbase @ ADDRSTACKcurrenttop @ + @ \ n currentheapaddr
    swap \ currentheapaddr n
    ADDRSTACKcurrenttop @ \ currentheapaddr n currenttop
    swap                  \ currentheapaddr currenttop n
    4 *                   \ currentheapaddr currenttop n*4
    -                     \ currentheapaddr newtop
    dup                   \ currentheapaddr newtop newtop
    0< if                 \ currentheapaddr newtop
        drop
        0                 \ currentheapaddr 0
    then
    ADDRSTACKcurrenttop ! \ currentheapaddr
    ADDRSTACKcurrentbase @ ADDRSTACKcurrenttop @ + ! \ ()
;

\
\ Dump stack management
\

2048 constant DUMPSTACKtotalsize
create DUMPSTACK DUMPSTACKtotalsize allot
variable DUMPSTACKcurrenttop

: initializeDumpStack
    8 negate DUMPSTACKcurrenttop !
;

\ dumpStackDepth inserts the depth of the dump stack onto the stack
\ ( -- dumpdepth )
: dumpStackDepth
    DUMPSTACKcurrenttop @ \ currenttop
    0< if
        0
    else
        DUMPSTACKcurrenttop @ \ currenttop
        8 / 1 +
    then
;

: dumpStackPush
    DUMPSTACKtotalsize 16 -
    DUMPSTACKcurrenttop @
    < if
        throwDumpStackFull throw
    then
    DUMPSTACKcurrenttop @ 8 + DUMPSTACKcurrenttop !
    DUMPSTACK DUMPSTACKcurrenttop @ + ! \ addrStackCurrentTop
    DUMPSTACK DUMPSTACKcurrenttop @ 4 + + ! 
;

\ dumpStackPop is designed to pop an entry from the dump stack
\ ( -- addrStackCurrentTop addrStackCurrentBase )
: dumpStackPop
    DUMPSTACKcurrenttop @
    0< if
        throwDumpStackEmpty throw
    else
        DUMPSTACK DUMPSTACKcurrenttop @ 4 + + @ \ addrStackCurrentTop
        DUMPSTACK DUMPSTACKcurrenttop @ + @
        \ addrStackCurrentTop addrStackCurrentBase
        DUMPSTACKcurrenttop @ 8 - DUMPSTACKcurrenttop !
        \ addrStkCurrTop addrStkCurrBase
    then
;

\ dumpStackPeek peeks at the nth stack element where n is on the stack
\ ( nth -- currenttop currentbase )
: dumpStackPeek
    DUMPSTACKcurrenttop @ \ nth currenttop
    swap                  \ currenttop nth
    8 *                   \ currenttop nth*8
    -                     \ grabfromhere
    dup                   \ grabfromhere grabfromhere
    0< if
        throwDumpInvalidPeek throw
    then
    dup                   \ grabfromhere grabfromhere
    4 + DUMPSTACK + @     \ grabfromhere currenttop
    swap                  \ currenttop grabfromhere
    DUMPSTACK + @         \ currenttop currentbase
;

\
\  Instruction stack management.  The instruction stack is where
\  instructions are kept that are waiting to run.
\

2048 constant INSTRSTACKtotalsize
create INSTRSTACK INSTRSTACKtotalsize allot
variable INSTRSTACKcurrenttop

: initializeInstrStack
    4 negate INSTRSTACKcurrenttop !
;

\ ( instr -- )
: instrStackPush
    INSTRSTACKtotalsize 16 -
    INSTRSTACKcurrenttop @
    < if
        throwInstrStackFull throw
    then
    INSTRSTACKcurrenttop @ 4 + INSTRSTACKcurrenttop !
    INSTRSTACK INSTRSTACKcurrenttop @ + ! \ addrStackCurrentTop
;

\ ( -- instr )
: instrStackPop
    INSTRSTACKcurrenttop @
    0< if
        throwInstrStackEmpty throw
    else
        INSTRSTACK INSTRSTACKcurrenttop @ + @
        INSTRSTACKcurrenttop @ 4 - INSTRSTACKcurrenttop !
    then
;

\
\ Heap management
\

\ Heap flags 
hex
80000000 constant GMHEAPforwardbiton
7fffffff constant GMHEAPforwardbitoff
decimal

\ Global Heap
2048 constant GMGLOBALHEAPtotalsize
create GMGLOBALHEAPbuffer GMGLOBALHEAPtotalsize allot
variable GMGLOBALHEAP
variable GMGLOBALHEAPnextunused

\ Dynamic Heap
32768 constant GMHEAPtotalsize
GMHEAPtotalsize 12 / constant GMHEAPmaxelements
create GMHEAPbuffer GMHEAPtotalsize 2 * allot
variable GMHEAP
variable GMHEAPactive
variable GMHEAPinactive
variable GMHEAPnextunused
variable GMHEAPnextunusedinactive
variable GMHEAPscavengeindex
-1 constant GMHEAPillegaladdress

\
\  Standard heap routines.
\

\ initializeHeap creates an initial heap named GMHEAP.
\ ( -- )
: initializeHeap
    0 GMGLOBALHEAPnextunused !
    GMGLOBALHEAPbuffer GMGLOBALHEAP !
    
    GMHEAPbuffer GMHEAP !
    0 GMHEAPnextunused !
    0 GMHEAPnextunusedinactive !

    0 GMHEAPscavengeindex !
    
    0 GMHEAPactive !
    GMHEAPtotalsize GMHEAPinactive !
;

\ hGlobalAlloc reserves heap space for a global definition that will
\ never be removed from the heap.  It uses the global heap.
\ ( [instrs] numInstrs -- heapAddr )
: hGlobalAlloc
    GMGLOBALHEAPtotalsize 16 -
    GMGLOBALHEAPnextunused @
    < if
        throwHeapFull throw
    then
    dup                                   \ [instrs] numInstrs numInstrs
    GMGLOBALHEAP @ GMGLOBALHEAPnextunused @ + ! \ [instrs] numInstrs
    1 +                                   \ [instrs] numInstrs+1
    1                                     \ [instrs] numInstrs+1 1
    do                                    \ [instrs]
        GMGLOBALHEAP @ GMGLOBALHEAPnextunused @ + i 4 * + !
    loop
    GMGLOBALHEAPnextunused @                    \ nextUnused
    GMGLOBALHEAPnextunused @ 4 +                \ nextUnused nextunused+4
    GMGLOBALHEAP @ GMGLOBALHEAPnextunused @ + @       \ nextUnused nextunused+4 numInstrs 
    4 *                                   \ nextUnused nextunused+4 numInstrbytes
    +                                     \ nextUnused nextunused+4+numInstrbytes
    GMGLOBALHEAPnextunused !                    \ nextUnused
;

\ hGlobalLookup extracts a global definition from the heap.
\ ( heapAddr -- )
: hGlobalLookup
    GMGLOBALHEAP @ +            \ heapAddr
    dup                         \ heapAddr heapAddr
    @                           \ heapAddr numInstrs
    1 +                         \ heapAddr numInstrs+1
    1                           \ heapAddr numIntsrs+1 1
    do                          \ heapAddr
        dup                     \ heapAddr heapAddr
        i 4 * + @               \ heapAddr instr
        instrStackPush          \ heapAddr
    loop
    drop                        \
;

\ hSize returns the number of entries in the current heap.
\ ( -- numElements )
: hSize
    GMHEAPnextunused @ \ numBytes
    12 /               \ numElements
;

\ hAlloc assigns a Node into the heap.
\ ( nodeLeft nodeRight NodeType -- heapAddr )
\ ( <value> <ignored> NNum -- heapAddr )
\ ( <heapaddr> <heapaddr> NAp -- heapAddr )
\ ( <numvars> <heapAddr> NGlobal -- heapAddr )
\ ( <heapaddr> <ignored> NInd -- heapAddr )
: hAlloc
    GMHEAPtotalsize 16 -
    GMHEAPnextunused @
    < if
        throwHeapFull throw
    then
    GMHEAPactive @ GMHEAP @ + GMHEAPnextunused @ + !      \ put nodeType into heap
    GMHEAPactive @ GMHEAP @ + GMHEAPnextunused @ + 8 + !  \ put nodeRight into heap
    GMHEAPactive @ GMHEAP @ + GMHEAPnextunused @ + 4 + !  \ put nodeLeft into heap
    GMHEAPactive @ GMHEAPnextunused @ +        \ put heap address of node onto stack
    GMHEAPnextunused @ 12 + GMHEAPnextunused !
;

\ hLookup extracts from a heap address the node triple that resides there
\ ( heapAddr -- nodeLeft nodeRight nodeType )
: hLookup
    GMHEAP @ +    \ heapAddr
    dup                            \ heapAddr heapAddr
    dup                            \ heapAddr heapAddr heapAddr
    @                              \ heapAddr heapAddr nodeType
    swap                           \ heapAddr nodeType heapAddr
    4 + @                          \ heapAddr nodeType nodeLeft
    rot                            \ nodeType nodeLeft heapAddr
    8 + @                          \ nodeType nodeLeft nodeRight
    rot                            \ nodeLeft nodeRight nodeType
;    

\ hPeek nondestructively looks at the current nodeType in the heap
\ ( heapAddr -- heapAddr nodeType )
: hPeek
    dup                            \ heapAddr heapAddr
    GMHEAP @ +                     \ heapAddr heapAddr
    @
;

\ hUpdate destructively modifies an already existing heap node.
\ This is used for lazy evaluation support via indirections.
\ ( nodeLeft nodeRight NodeType heapAddr -- )
: hUpdate
    GMHEAP @ +  \ nodeLeft nodeRight nodeType heapAddr
    >r             \ nodeLeft nodeRight nodeType
    r@             \ nodeLeft nodeRight nodeType heapAddr
    !              \ nodeLeft nodeRight
    r@ 8 +         \ nodeLeft nodeRight heapAddr
    !              \ nodeLeft
    r@ 4 +         \ nodeLeft heapAddr
    !              \ ()
    r> drop        \ ()
;

\ ( heapAddr -- )
: dumpHeapElement
    dup \ heapAddr heapAddr
    hLookup \ heapAddr nodeLeft nodeRight nodeType
    s" <" type
    s" type=" type
    u.
    swap
    s" left=" type
    .
    s" right=" type
    .
    s" addr=" type
    .
    s" >" type
;

\ This function dumps a mapping from the current active heap to the
\ (about to be active) inactive heap.
\ ( heapAddr -- )
: dumpHeapLeap
    dup dumpHeapElement
    s"  -> " type 
    hLookup
    2drop
    dumpHeapElement 
;

\ ( heapAddr -- )
: dumpHeapReach
    dup dumpHeapElement s"  " type
    hLookup \ l r t
    case
        NAp of
            swap \ r l
            s"  -> ( " type
            recurse
            s" , " type
            recurse
            s"  ) " type
        endof
        NInd of
            drop
            s"  -> ( " type
            recurse
            s"  ) " type
        endof
        2drop
    endcase
;

( -- )
: heapReachability
    addrStackDepth
    0
    do
        i addrStackPeek dumpHeapReach cr
    loop
;

\
\  Heap Garbage Collection Routines
\

\ Used after garbage collection is complete, switchCurrentHeap switches
\ the heaps - using the new compacted one.
\ ( -- )
: hSwitchHeaps
    GMHEAPactive @
    0
    = if
        0 GMHEAPinactive !
        GMHEAPtotalsize GMHEAPactive !
    else
        0 GMHEAPactive !
        GMHEAPtotalsize GMHEAPinactive !
    then
    GMHEAPnextunusedinactive @ GMHEAPnextunused !
    0 GMHEAPnextunusedinactive !
    0 GMHEAPscavengeindex !
;

\ setForwardBit sets the forward bit, and stores the forward pointer
\ in the nodeLeft component of the heap element.  The forward bit is
\ used during the stop and copy garbage collection.
\ ( newHeapAddr oldHeapAddr -- newHeapAddr )
: setForwardBit
    dup dup                  \ newHeapAddr oldHeapAddr oldHeapAddr oldHeapAddr
    GMHEAP @ + @             \ newHeapAddr oldHeapAddr oldHeapAddr nodeType
    GMHEAPforwardbiton or    \ newHeapAddr oldHeapAddr oldHeapAddr newNodeType
    swap                     \ newHeapAddr oldHeapAddr newNodeType oldHeapAddr
    GMHEAP @ + !             \ newHeapAddr oldHeapAddr
    swap                     \ oldHeapAddr newHeapAddr
    dup                      \ oldHeapAddr newHeapAddr newHeapAddr
    rot                      \ newHeapAddr newHeapAddr oldHeapAddr
    GMHEAP @
    +                        \ newHeapAddr newHeapAddr 
    4 +
    !                        \ newHeapAddr
;

\ ( nodeType -- boolean )
: isForwardBitSet
    GMHEAPforwardbiton and
    if
        true
    else
        false
    then
;

\ hCopyToInactive copies a node into the inactive heap.  Used during garbage collection.
\ ( nodeLeft nodeRight NodeType -- heapAddr )
\ ( <value> <ignored> NNum -- heapAddr )
\ ( <heapaddr> <heapaddr> NAp -- heapAddr )
\ ( <numvars> <heapAddr> NGlobal -- heapAddr )
: hCopyToInactive
    GMHEAPtotalsize 16 -
    GMHEAPnextunusedinactive @
    < if
        throwHeapFull throw
    then
    GMHEAPinactive @ GMHEAP @ + GMHEAPnextunusedinactive @ + !      \ put nodeType into heap
    GMHEAPinactive @ GMHEAP @ + GMHEAPnextunusedinactive @ + 8 + !  \ put nodeRight into heap
    GMHEAPinactive @ GMHEAP @ + GMHEAPnextunusedinactive @ + 4 + !  \ put nodeLeft into heap
    GMHEAPinactive @ GMHEAPnextunusedinactive @ +                 \ heapAddr
    GMHEAPnextunusedinactive @ 12 + GMHEAPnextunusedinactive !
;

\ hEvacuateNode evacuates a node to the (currently) inactive heap.
\ This is the first of two parts of garbage collection.  The second
\ part is the scavenger.  See section 2.9.4 of Peyton-Jones/Lester
\ ( oldHeapAddr -- newHeapAddr )
: hEvacuateNode
    dup \ oldHeapAddr oldHeapAddr
    hLookup \ oldHeapAddr nodeLeft nodeRight nodeType
    dup
    isForwardBitSet \ oldHeapAddr nodeLeft nodeRight nodeType bool
    if
        drop \ oldHeapAddr nodeLeft nodeRight
        drop \ oldHeapAddr nodeLeft
        nip  \ nodeLeft
    else \ oldHeapAddr nodeLeft nodeRight nodeType
        hCopyToInactive \ oldHeapAddr newHeapAddr
        swap            \ newHeapAddr oldHeapAddr
        setForwardBit   \ newHeapAddr
    then
;

\ hScavenge is designed to walk the new heap and pull nodes from the
\ old heap into it.
\ ( -- )
: hScavenge
    begin
        GMHEAPinactive @ GMHEAPnextunusedinactive @ + \ nextinactiveAddr
        GMHEAPinactive @ GMHEAPscavengeindex @ +      \ nextinactiveAddr scavengeAddr
        dup \ nextinactiveAddr scavengeAddr scavengeAddr
        >r  \ nextinactiveAddr scavengeAddr
        <>  \ ()
    while
            r@ \ scavengeAddr
            hLookup \ nodeLeft nodeRight nodeType
            case
                NInd of \   nodeLeft nodeRight
                    swap \   nodeRight nodeLeft
                    hEvacuateNode \   nodeRight newHeapAddr
                    swap \   newHeapAddr nodeRight
                    NInd \   newHeapAddr nodeRight NInd
                    r@ \ newHeapAddr nodeRight NInd heapAddr
                    hUpdate 
                endof
                NAp of        \ nodeLeft nodeRight
                    hEvacuateNode \ nodeLeft newRightHeapAddr
                    swap \ newRightHeapAddr nodeLeft
                    hEvacuateNode \ newRightHeapAddr newLeftHeapAddr
                    swap \ newLeftHeapAddr newRightHeapAddr
                    NAp  \ newHeapAddr nodeRight NAp
                    r@    \ newHeapAddr nodeRight NAp heapAddr
                    hUpdate
                endof
                2drop \ default case
            endcase
            r> drop
            GMHEAPscavengeindex @ 12 + GMHEAPscavengeindex !
    repeat
    r> drop
;

\ hGarbageCollect does the garbage collection of the heap.  It's a
\ basic stop and copy implementation.  We basically traverse the
\ address stack looking for heap nodes that are still reachable by the
\ program.  We then scavenge the new heap looking for nodes still
\ (indirectly) reachable.
\ ( -- )
: hGarbageCollect
    CollectGarbageLight turnOnLight
    \ handle the globals map table
    aSize
    0
    2dup <> if
        do
            i aFetch \ globalnum oldHeapAddr
            hEvacuateNode \ globalnum newHeapAddr
            swap \ newHeapAddr globalnum
            aUpdate \ ()
        loop
    else
        2drop
    then
    \ deal with the addresses on the address stack.
    addrStackTotalDepth
    0
    2dup <> if
        do
            i addrStackTotalPeek \ heapAddr
            hEvacuateNode   \ newHeapAddr
            i addrStackTotalOverwrite \ ()
        loop
    else
        2drop
    then
    hScavenge
    hSwitchHeaps
;

\
\ Handy debugging routines.
\

\ addrStackDisplay prints the contents of the address stack
\ ( -- )
: addrStackDisplay
    addrStackDepth \ stackdepth
    0
    2dup <> if
        do \ ?do
        i addrStackPeek hPeek \ heapAddr Nodetype
        s" [" type
        s" type=" type
        u.
        s" addr=" type
        u.
        s" ]" type
        loop
        cr
    else
        s" [ADDR STACK EMPTY]" type cr
        2drop
    then
;

\ dumpStackDisplay prints the contents of the dump stack
\ ( -- )
: dumpStackDisplay
    dumpStackDepth \ stackdepth
    0
    2dup <> if
        do \ ?do
        i dumpStackPeek
        s" {" type
        s" base=" type
        u.
        s" top=" type
        u.
        s" }" type
        loop
        cr
    else
        s" [DUMP STACK EMPTY]" type cr
        2drop
    then
;

\
\  Now starts G-machine related code.
\

\ handleArguments
\ this function is used by unwind to point directly to
\ arguments, instead of having to go through application nodes.
\ ( nodeLeft nodeRight -- )
: handleArguments
    2dup
    drop \ nodeLeft nodeRight nodeLeft
    true \ nodeLeft nodeRight nodeLeft true
    swap \ nodeLeft nodeRight true nodeLeft
    1 +  \ nodeLeft nodeRight true nodeLeft+1
    addrStackDepth \ nodeLeft nodeRight true nodeLeft+1 stackDepth
    > if
        drop
        false
    then

    \ now there is either true or false on the stack
    \ true represents the case where there are enough
    \ arguments on the stack to do a stack transformation
    if   \ nodeLeft nodeRight
        2dup   \ nodeLeft nodeRight nodeLeft nodeRight
        drop 1 +      \ nodeLeft+1
        1             \ nodeLeft+1 1
        2dup <> if
            do \ ?do
                i addrStackPeek
                hLookup         \ nodeLeft nodeRight nodeType
                drop
                nip             \ nodeRight
                i 1 -           \ i-1 
                addrStackOverwrite
            loop
        else
            2drop
        then
    then
    \ nodeLeft nodeRight
;

\ ( )
: dounwind
    0 addrStackPeek \ heapAddr
    hLookup  \ nodeLeft nodeRight nodeType
    case
        NNum of \ nodeLeft nodeRight
            dumpStackDepth \ nodeLeft nodeRight depth
            0= if
                2drop   \ ()
                EndOfCode instrStackPush \ EndOfCode
            else
                2drop    \ ()
                addrStackPop \ heapAddr
                dumpStackPop \ heapAddr addrStkCurrtop addrStkCurrBase
                addrStackPutBaseAndCurrent \ heapAddr
                addrStackPush \ ()
            then
        endof
        NInd of \ nodeLeft nodeRight
            drop  \ nodeLeft
            0 addrStackOverwrite \ ()
            Unwind instrStackPush
        endof
        NAp of        \ nodeLeft nodeRight
            drop      \ nodeLeft
            addrStackPush \ ()
            Unwind instrStackPush
        endof
        NGlobal of    \ nodeLeft nodeRight
            handleArguments \ nodeLeft nodeRight
            nip       \ nodeRight
            hGlobalLookup \ ()
        endof
    endcase
;

\ ( -- )
: dopushglobal
    instrStackPop \ globalnum
    aLookup    \ heapAddr
    addrStackPush \ ()
;

\ ( -- )
: dopushint
    instrStackPop \ i
    0 \ i 0
    NNum \ i 0 NNum
    hAlloc \ heapAddr
    addrStackPush \ ()
;

\ ( -- )
: dopush
    instrStackPop    \ n
    addrStackPeek    \ heapAddr
    addrStackPush    \ ()
;

\ ( -- )
: domkap
    addrStackPop \ f
    addrStackPop \ f a
    NAp          \ f a NAp
    hAlloc       \ heapAddr
    addrStackPush \ ()
;

\ ( -- )
: doupdate
    instrStackPop    \ n
    >r               
    addrStackPop     \ targetHeapAddr
    0                \ targetHeapAddr 0
    NInd             \ targetHeapAddr 0 NInd
    r@               \ targetHeapAddr 0 NInd n
    addrStackPeek    \ targetHeapAddr 0 NInd heapAddr
    dup >r           \ targetHeapAddr 0 NInd heapAddr
    hUpdate          \ ()
    r>               \ heapAddr
    r>               \ heapAddr n
    addrStackOverwrite
;

\ ( -- )
: dopop
    instrStackPop  \ n
    dup ( n n )
    0> if ( n )
        0    \ n 0
        do   \ ()
            addrStackPop
            drop
        loop
    else ( n )
        drop
    then
;

\ ( -- )
: doalloc
    instrStackPop  \ n
    0
    do
        GMHEAPillegaladdress
        0
        NInd           \ GMHEAPillegaladdress 0 NInd
        hAlloc         \ heapAddr
        addrStackPush  \ ()
    loop
;

\ ( -- )
: doslide
    instrStackPop \ n
    addrStackSlide \ ()
;

\ ( -- )
: doeval
    addrStackPop \ heapAddr
    addrStackGetBaseAndCurrent \ heapAddr addrStkCurrTop addrStkCurrBase
    dumpStackPush \ heapAddr
    addrStackReInit \ heapAddr
    addrStackPush \ ()
    Unwind instrStackPush
;

\ boxInteger drops a new integer into the heap
\ ( integervalue -- heapAddr )
: boxInteger
    0
    NNum
    hAlloc \ heapAddr
;

\ unBoxInteger removes an integer from a heap node.
\ ( heapNode -- integervalue )
: unBoxInteger
    hLookup \ nodeLeft nodeRight nodeType
    NNum
    <> if
        throwUnboxIntProblem throw
    then
    drop    \ nodeLeft
;

: doadd
    addrStackPop unBoxInteger \ leftInteger
    addrStackPop unBoxInteger \ rightInteger
    + \ sumInteger
    boxInteger \ heapAddr
    addrStackPush \ ()
;

: dosub
    addrStackPop unBoxInteger \ leftInteger
    addrStackPop unBoxInteger \ rightInteger
    - \ differenceInteger
    boxInteger \ heapAddr
    addrStackPush \ ()
;

: domul
    addrStackPop unBoxInteger \ leftInteger
    addrStackPop unBoxInteger \ rightInteger
    * \ productInteger
    boxInteger \ heapAddr
    addrStackPush \ ()
;

: dodiv
    addrStackPop unBoxInteger \ leftInteger
    addrStackPop unBoxInteger \ rightInteger
    / \ divInteger
    boxInteger \ heapAddr
    addrStackPush \ ()
;

: doneg
    addrStackPop unBoxInteger \ leftInteger
    negate \ negatedInteger
    boxInteger \ heapAddr
    addrStackPush \ ()
;

\ boxBoolean
\ ( booleanvalue -- heapAddr )
: boxBoolean
    if
        1
    else
        0
    then
    0
    NNum
    hAlloc \ heapAddr
;

\ ( heapAddr -- boolean value)
: unBoxBoolean
    hLookup \ nodeLeft nodeRight nodeType
    NNum
    <> if
        throwUnboxBooleanProblem throw
    then
    drop    \ nodeLeft
    1
    = if
        true \ true
    else
        false \ false
    then
;

: doEq
    addrStackPop unBoxInteger \ leftInteger
    addrStackPop unBoxInteger \ rightInteger
    =
    boxBoolean \ box the true or false
    addrStackPush \ ()
;

: doNe
    addrStackPop unBoxInteger \ leftInteger
    addrStackPop unBoxInteger \ rightInteger
    <>
    boxBoolean \ box the true or false
    addrStackPush \ ()
;

: doLt
    addrStackPop unBoxInteger \ leftInteger
    addrStackPop unBoxInteger \ rightInteger
    <
    boxBoolean \ box the true or false
    addrStackPush \ ()
;

: doLe
    addrStackPop unBoxInteger \ leftInteger
    addrStackPop unBoxInteger \ rightInteger
    > 0=
    boxBoolean \ box the true or false
    addrStackPush \ ()
;

: doGt
    addrStackPop unBoxInteger \ leftInteger
    addrStackPop unBoxInteger \ rightInteger
    >
    boxBoolean \ box the true or false
    addrStackPush \ ()
;

: doGe
    addrStackPop unBoxInteger \ leftInteger
    addrStackPop unBoxInteger \ rightInteger
    < 0=
    boxBoolean \ box the true or false
    addrStackPush \ ()
;

\ The cond is structured as follows:
\
\ <instrs for false>
\ <length of false instrs>
\ <purge>
\ <instrs for true>
\ <length of true instrs>
\ <Cond>
\
\ If true case occurs, the stream is simply executed.
\ If false occurs, the true stream is dropped, along with the
\ purge (and its associated length) in front of the false instrs.
\ ( -- )
: docond
    addrStackPop  \ heapAddr
    unBoxBoolean
    if
        instrStackPop \ truebranchdepth
        drop
    else
        instrStackPop \ truebranchdepth
        2 + \ truebranchdepth+2 (skip true instructions and purge n)
        0
        do
            instrStackPop
            drop
        loop
    then
;

\ ( -- )
: dopurge
    instrStackPop \ inststopurge
    0
    do
        instrStackPop
        drop
    loop
;

\ G Machine evaluator section
\ ( -- )
variable EVALnumsteps
: eval
    0 EVALnumsteps !
    begin
        instrStackPop
        dup
        EndOfCode <>
    while
            \ dup .
            case                 \ ()
                Unwind of
                    UnwindLight turnOnLight
                    dounwind
                endof
                Pushglobal of
                    PushglobalLight turnOnLight
                    dopushglobal
                endof
                Pushint of
                    PushintLight turnOnLight
                    dopushint
                endof
                Push of
                    PushLight turnOnLight
                    dopush
                endof
                Mkap of
                    MkapLight turnOnLight
                    domkap
                endof
                UpdateI of
                    UpdateLight turnOnLight
                    doupdate
                endof
                Pop of
                    PopLight turnOnLight
                    dopop
                endof
                Alloc of
                    doalloc
                endof
                Slide of
                    doslide
                endof
                EvalI of
                    doeval
                endof
                Add of
                    doadd
                endof
                Sub of
                    dosub
                endof
                Mul of
                    domul
                endof
                Div of
                    dodiv
                endof
                Neg of
                    doneg
                endof
                Eq of
                    doEq
                endof
                Ne of
                    doNe
                endof
                Lt of
                    doLt
                endof
                Le of
                    doLe
                endof
                Gt of
                    doGt
                endof
                Ge of
                    doGe
                endof
                Cond of
                    docond
                endof
                Purge of
                    dopurge
                endof
            endcase
            \ addrStackDisplay
            \ dumpStackDisplay
            \ dumpStackDepth .
            EVALnumsteps @ 1 + EVALnumsteps !
            \ Now do garbage collection
            hSize
            GMHEAPmaxelements 64 -
            < 0= if
                hGarbageCollect
            then
            \ display heap nodes on LED
            hSize 1048584 w!
    repeat \ EndOfCode
    drop \ ()
    EndOfCodeLight turnOnLight
    EVALnumsteps @ \ dup . \ numbersteps
    addrStackPop \ numbersteps heapAddr
    hLookup      \ numbersteps nodeLeft nodeRight nodeType
    drop drop \ 2dup . . \ numbersteps nodeLeft
    \ cr s" heapsize= " type hSize . cr
    \ numbersteps nodeLeft
;

\ End of ForthRTS.fs

initializeHeap
initializeGlobals
initializeAddrStack
initializeDumpStack
initializeInstrStack

2
Push 1
EvalI
Push 1
EvalI
Add
UpdateI 2
Pop 2
Unwind
12
hGlobalAlloc
NGlobal
hAlloc
1
aInsert

2
Push 1
EvalI
Push 1
EvalI
Sub
UpdateI 2
Pop 2
Unwind
12
hGlobalAlloc
NGlobal
hAlloc
2
aInsert

2
Push 1
EvalI
Push 1
EvalI
Mul
UpdateI 2
Pop 2
Unwind
12
hGlobalAlloc
NGlobal
hAlloc
3
aInsert

2
Push 1
EvalI
Push 1
EvalI
Div
UpdateI 2
Pop 2
Unwind
12
hGlobalAlloc
NGlobal
hAlloc
4
aInsert

1
Push 0
EvalI
Neg
UpdateI 1
Pop 1
Unwind
9
hGlobalAlloc
NGlobal
hAlloc
5
aInsert

2
Push 1
EvalI
Push 1
EvalI
Eq
UpdateI 2
Pop 2
Unwind
12
hGlobalAlloc
NGlobal
hAlloc
6
aInsert

2
Push 1
EvalI
Push 1
EvalI
Ne
UpdateI 2
Pop 2
Unwind
12
hGlobalAlloc
NGlobal
hAlloc
7
aInsert

2
Push 1
EvalI
Push 1
EvalI
Lt
UpdateI 2
Pop 2
Unwind
12
hGlobalAlloc
NGlobal
hAlloc
8
aInsert

2
Push 1
EvalI
Push 1
EvalI
Le
UpdateI 2
Pop 2
Unwind
12
hGlobalAlloc
NGlobal
hAlloc
9
aInsert

2
Push 1
EvalI
Push 1
EvalI
Gt
UpdateI 2
Pop 2
Unwind
12
hGlobalAlloc
NGlobal
hAlloc
10
aInsert

2
Push 1
EvalI
Push 1
EvalI
Ge
UpdateI 2
Pop 2
Unwind
12
hGlobalAlloc
NGlobal
hAlloc
11
aInsert

3
Push 0
EvalI
Cond
2
Push 1
Purge
2
Push 2
UpdateI 3
Pop 3
Unwind
16
hGlobalAlloc
NGlobal
hAlloc
12
aInsert

1
Pushint 2
Push 1
Pushglobal 2
Mkap
Mkap
Pushglobal 13
Mkap
Pushint 1
Push 2
Pushglobal 2
Mkap
Mkap
Pushglobal 13
Mkap
Pushglobal 1
Mkap
Mkap
Pushint 1
Pushint 2
Push 3
Pushglobal 9
Mkap
Mkap
Pushglobal 12
Mkap
Mkap
Mkap
UpdateI 1
Pop 1
Unwind
46
hGlobalAlloc
NGlobal
hAlloc
13
aInsert

0
Pushint 15
Pushglobal 13
Mkap
UpdateI 0
Pop 0
Unwind
10
hGlobalAlloc
NGlobal
hAlloc
14
aInsert

Pushglobal 14
EvalI
EndOfCode
instrStackPush
instrStackPush
instrStackPush
instrStackPush

