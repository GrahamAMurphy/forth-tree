\ $Id: ForthRTS.fs,v 1.46 2005/08/15 02:23:07 andrew Exp $

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
    \ 100002 w!
    drop
;

\ ( testpointNumber - )
: setTestpoint
\     dup \ tpNum tpNum
\     1 > if \ tpNum
\         1 - 2 *  \ pattern
\     then
\     10000a @ \ pattern currentTP
\     or
\     10000a w!
    drop \ REMOVE 
    
;

\ ( testpointNumber - )
: clearTestpoint
\     dup \ tpNum tpNum
\     1 > if \ tpNum
\         1 - 2 *  \ pattern
\     then
\     invert \ pattern
\     10000a @ \ pattern currentTP
\     and
\     10000a w!
    drop \ REMOVE
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
\ nodeType nodeLeft nodeRight 
1 constant NNum  \ heap elements that are just numbers
2 constant NAp   \ heap elements that are function applications
3 constant NGlobal \ heap nodes that point to supercombinator code defns
4 constant NInd  \ indirection nodes for lazy evaluation
5 constant NConstr \ constructor node

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

create INSTRscratchbuffer 64 allot
variable INSTRnextaddress

\ ( -- instruction )
: getNextInstruction
    INSTRnextaddress @ @
    INSTRnextaddress @ 4 + INSTRnextaddress !
    \ dup . \ DEBUG
;

\ ( address -- )
: setInstructionPointer
    INSTRnextaddress !
;

\ ( -- address )
: getInstructionPointer
    INSTRnextaddress @
;

\ ( [instrs] num -- )
: injectInstructions
    0                                     \ [instrs] numInstrs 0
    swap                                  \ [instrs] 0 numInstrs
    1 -                                   \ [instrs] 0 numInstrs-1
    do                                    \ [instrs]
        INSTRscratchbuffer i 4 * + !
    1 negate +loop
    INSTRscratchbuffer INSTRnextaddress !
;

\
\ Old instruction stack stuff
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
24000 constant GMHEAPtotalsize
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

\ ( heapAddr - addr )
: absoluteGlobalHeapAddress
    GMGLOBALHEAP @ + 
;

\ hGlobalBytesRemaining returns the number of available bytes in the global heap.
\ ( -- bytesremaining )
: hGlobalBytesRemaining
   GMGLOBALHEAPtotalsize GMGLOBALHEAPnextunused @ - 
;

\ hGlobalAlloc reserves heap space for a global definition that will
\ never be removed from the heap.  It uses the global heap.
\ ( [instrs] numInstrs -- heapAddr )
: hGlobalAlloc
    4 * \ number of instructions in bytes
    4 + \ plus the number of instructions field
    hGlobalBytesRemaining
    > if
        throwHeapFull throw
    then
    dup                                   \ [instrs] numInstrs numInstrs
    GMGLOBALHEAP @ GMGLOBALHEAPnextunused @ + ! \ [instrs] numInstrs
    1                                     \ [instrs] numInstrs 1
    swap                                  \ [instrs] 1 numInstrs
    do                                    \ [instrs]
        GMGLOBALHEAP @ GMGLOBALHEAPnextunused @ + i 4 * + !
    1 negate +loop
    GMGLOBALHEAPnextunused @                    \ nextUnused
    GMGLOBALHEAPnextunused @ 4 +                \ nextUnused nextunused+4
    GMGLOBALHEAP @
    GMGLOBALHEAPnextunused @ + @  \ nextUnused nextunused+4 numInstrs 
    4 *                                   \ nextUnused nextunused+4 numInstrbytes
    +                                     \ nextUnused nextunused+4+numInstrbytes
    GMGLOBALHEAPnextunused !                    \ nextUnused
;

\ hBytesRemaining returns the number of bytes of heap available for use.
\ ( -- bytesRemaining )
: hBytesRemaining
    GMHEAPtotalsize GMHEAPnextunused @ - \ numavailablebytes
;

\ hAllocNConstrComponents allocates the constructor components heap element
\ ( [addrs] <numaddrs> -- heapAddr )
: hAllocNConstrComponents
    dup \ [addrs] <numaddrs> <numaddrs>
    4 * 12 + \ [addrs] <numaddrs> bytesrequired
    hBytesRemaining
    > if
        throwHeapFull throw
    then \ [addrs] <numaddrs>
    >r
    1 \ [addrs] 1
    r@ \ [addrs] 1 <numaddrs>
    2dup <> if
        do \ ?do \ [addrs]
            GMHEAPactive @ GMHEAP @ + GMHEAPnextunused @ + i 4 * + !
        1 negate +loop
    else
        2drop
    then
    r@                                         \ numaddrs
    GMHEAPactive @ GMHEAP @ + GMHEAPnextunused @ + ! \ ()
    GMHEAPactive @ GMHEAPnextunused @ +        \ heapAddr
    GMHEAPnextunused @ 4 +                     \ heapaddr heapSkipNumAddrs
    r>
    4 *                                    \ heapAddr heapSkipNumAddrs numAddrBytes
    +                                          \ heapAddr heapNowOccupied
    GMHEAPnextunused !                         \ heapAddr
;

\ hAlloc assigns a Node into the heap.
\ ( nodeLeft nodeRight NodeType -- heapAddr )
\ ( <value> <ignored> NNum -- heapAddr )
\ ( <heapaddr> <heapaddr> NAp -- heapAddr )
\ ( <numvars> <heapAddr> NGlobal -- heapAddr )
\ ( <heapaddr> <ignored> NInd -- heapAddr )
\ ( [addrs] <numaddrs> <tagvalue> NConstr -- heapAddr )
: hAlloc
    >r
    r@
    NConstr = if
        \ we need to allocate the NConstrComps section.
        \ [addrs] <numaddrs> <tagvalue>
        >r \ [addrs] <numaddrs>
        hAllocNConstrComponents \ heapAddr
        r> \ heapAddr tagvalue
    then
    12
    hBytesRemaining
    > if
        throwHeapFull throw
    then
    r>
    GMHEAPactive @ GMHEAP @ + GMHEAPnextunused @ + !      \ put nodeType into heap
    GMHEAPactive @ GMHEAP @ + GMHEAPnextunused @ + 8 + !  \ put nodeRight into heap
    GMHEAPactive @ GMHEAP @ + GMHEAPnextunused @ + 4 + !  \ put nodeLeft into heap
    GMHEAPactive @ GMHEAPnextunused @ +      \ put heap address of node onto stack
    GMHEAPnextunused @ 12 + GMHEAPnextunused !
;

\ ( heapAddr -- [addrs] numaddrs )
: hLookupNConstrComponents
    GMHEAP @ +    \ heapAddr
    dup           \ heapAddr heapAddr
    @             \ heapAddr numaddrs
    >r                         \ heapAddr 
    r@ 1 +                     \ heapAddr numaddrs+1
    1                          \ heapAddr numaddrs+1 1
    2dup <> if
        do \ ?do
            dup                \ heapAddr heapAddr
            i 4 * + @          \ heapAddr addr
            swap               \ addr heapAddr
        loop
    else
        2drop
    then
    drop
    r>                          \ [addr] numaddrs
;

\ hLookup extracts from a heap address the node triple that resides there
\ ( heapAddr -- nodeLeft nodeRight nodeType )
\ ( heapAddr -- [addrs] numaddrs tag NConstr )
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
    dup                            \ nodeLeft nodeRight nodeType nodeType 
    NConstr = if
        >r >r                      \ nodeLeft
        hLookupNConstrComponents   \ [addrs] numaddrs
        r>                         \ [addrs] numaddrs tagvalue
        r>                         \ [addrs] numaddrs tagvalue NConstr
    then
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
\ Note that when an NConstr is updated, the old component array is
\ forgotten - it will be collected during garbage collection since it's
\ no longer reachable.
\           ( nodeLeft nodeRight NodeType heapAddr -- )
\ ( [addrs] <numaddrs> <tagvalue> NConstr heapAddr -- )
: hUpdate
    GMHEAP @ +  \ nodeLeft nodeRight nodeType heapAddr
    >r             \ nodeLeft nodeRight nodeType
    dup            \ nodeLeft nodeRight nodeType nodeType
    r@             \ nodeLeft nodeRight nodeType nodeType heapAddr
    !              \ nodeLeft nodeRight nodeType
    swap           \ nodeLeft nodeType nodeRight
    r@ 8 +         \ nodeLeft nodeType nodeRight heapAddr
    !              \ nodeLeft nodeType
    NConstr = if   \ if true: [addrs] <numaddrs>
        hAllocNConstrComponents \ heapAddr 
    then
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
    0<> if
        true
    else
        false
    then
;

\ hCopyToInactive copies a node into the inactive heap.  Used during
\ garbage collection.
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
    GMHEAPinactive @ GMHEAP @ +
    GMHEAPnextunusedinactive @ + !      \ put nodeType into heap
    GMHEAPinactive @ GMHEAP @ +
    GMHEAPnextunusedinactive @ + 8 + !  \ put nodeRight into heap
    GMHEAPinactive @ GMHEAP @ +
    GMHEAPnextunusedinactive @ + 4 + !  \ put nodeLeft into heap
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
        GMHEAPinactive @ GMHEAPscavengeindex @
        +      \ nextinactiveAddr scavengeAddr
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
\ ( nodeLeft nodeRight -- nodeLeft nodeRight bool)
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
        true
    else
        false
    then
    \ nodeLeft nodeRight bool
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
                EndOfCode 1 injectInstructions
            else
                2drop    \ ()
                addrStackPop \ heapAddr
                dumpStackPop \ heapAddr addrStkCurrtop addrStkCurrBase
                addrStackPutBaseAndCurrent \ heapAddr
                addrStackPush \ ()
                instrStackPop \ nextInstructionAddress
                setInstructionPointer \ ()
            then
        endof
        NInd of \ nodeLeft nodeRight
            drop  \ nodeLeft
            0 addrStackOverwrite \ ()
            Unwind 1 injectInstructions 
        endof
        NAp of        \ nodeLeft nodeRight
            drop      \ nodeLeft
            addrStackPush \ ()
            Unwind 1 injectInstructions
        endof
        NGlobal of    \ nodeLeft nodeRight
            1 setTestpoint
            handleArguments \ nodeLeft nodeRight bool
            if \ nodeLeft nodeRight
                nip       \ nodeRight
                4 + \ addrofFirstInstr
                absoluteGlobalHeapAddress setInstructionPointer \ ()
            else \ nodeLeft nodeRight
                2drop \ ()
                addrStackDepth
                1
                2dup <> if
                    do \ ?do
                        addrStackPop drop
                    loop
                else
                    2drop
                then
                addrStackPop \ heapAddr
                dumpStackPop \ heapAddr addrStkCurrtop addrStkCurrBase
                addrStackPutBaseAndCurrent \ heapAddr
                addrStackPush \ ()
                instrStackPop \ nextInstructionAddress
                setInstructionPointer \ ()
            then
            1 clearTestpoint
        endof
    endcase
;

\ ( -- )
: dopushglobal
    getNextInstruction \ n
    aLookup    \ heapAddr
    addrStackPush \ ()
;

\ ( -- )
: dopushint
    getNextInstruction \ n
    0 \ i 0
    NNum \ i 0 NNum
    hAlloc \ heapAddr
    addrStackPush \ ()
;

\ ( -- )
: dopush
    getNextInstruction \ n
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
    getNextInstruction \ n
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
    getNextInstruction \ n
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
    getNextInstruction \ n
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
    getNextInstruction \ n
    addrStackSlide \ ()
;

\ ( -- )
: doeval
    addrStackPop \ heapAddr
    addrStackGetBaseAndCurrent \ heapAddr addrStkCurrTop addrStkCurrBase
    dumpStackPush \ heapAddr
    addrStackReInit \ heapAddr
    addrStackPush \ ()
    getInstructionPointer \ nextInstructionAddress
    instrStackPush \ ()
    Unwind 1 injectInstructions
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

: doNEq
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
        getNextInstruction \ truebranchdepth
        drop
    else
        getNextInstruction \ truebranchdepth
        2 + \ truebranchdepth+2 (skip true instructions and purge n)
        0
        do
            getNextInstruction
            drop
        loop
    then
;

\ ( -- )
: dopurge
    getNextInstruction \ inststopurge
    0
    do
        getNextInstruction 
        drop
    loop
;

\ G Machine evaluator section
\ ( -- )
variable EVALnumsteps
: eval
    0 EVALnumsteps !
    begin
        getNextInstruction
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
                    doNEq
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
            hBytesRemaining
            32
            < if
                hGarbageCollect
            then
            \ hBytesRemaining 1048584 w!
    repeat \ EndOfCode
    drop \ ()
    EndOfCodeLight turnOnLight
    EVALnumsteps @ \ dup . \ numbersteps
    addrStackPop \ numbersteps heapAddr
    hLookup      \ numbersteps nodeLeft nodeRight nodeType
    drop drop \ 2dup . . \ numbersteps nodeLeft
    \ cr s" heap bytes remaining= " type hBytesRemaining . cr
    \ numbersteps nodeLeft
;

\ End of ForthRTS.fs

decimal

\
\  Globals tests. 
\

: testGlobalSimpleCase
    654 1 aInsert
    1 aLookup
    654 = if
        aSize
        1 = if
            s" [PASS]: testGlobalSimpleCase" type cr
        else
            s" [FAIL]: testGlobalSimpleCase FAILED 1" type cr
        then
    else
        s" [FAIL]: testGlobalSimpleCase FAILED 2" type cr
    then
    633 2 aInsert
    aSize
    2 = if
        s" [PASS]: test global list length" type cr
    else
        s" [FAIL]: test global list length 1" type cr
    then
    1024 2 aUpdate
    2 aLookup
    1024 = if
        s" [PASS]: aUpdate test" type cr
    else
        s" [FAIL]: aUpdate test 1" type cr
    then
    1 aFetch
    1024 = if
        2 = if
            s" [PASS]: aFetch test" type cr
        else
            s" [FAIL]: aFetch 1" type cr
        then
    else
        s" [FAIL]: aFetch 2" type cr
    then
;

: testGlobalsMaxOut
    1
    1
    do
        i i ['] aInsert catch
        throwGlobalsFull = if
            s" [PASS]: GLOBALS full on try: " type i . cr
            leave
        then
    loop
;

: testGlobalNotPresent
    1024 ['] aLookup catch 
    throwUnknownGlobal = if
        s" [PASS]: global not found" type cr
    else
        s" [FAIL]: global WAS found" type cr
    then
;

\
\  Addr Stack tests.
\

: testAddrTest1
    123 addrStackPush
    456 addrStackPush
    0 addrStackPeek
    456 = if
        s" [PASS]: addr stack peek" type cr
    else
        s" [FAIL]: addr stack peek" type cr
    then
    addrStackDepth
    2 = if
        s" [PASS]: addr stack depth" type cr
    else
        s" [FAIL]: addr stack depth" type cr
    then
    addrStackGetBaseAndCurrent \ current base
    13 17 addrStackPutBaseAndCurrent \ current base
    addrStackGetBaseAndCurrent \ current base 13 17
    17 = if
        13 = if
            s" [PASS]: addrStackGet/PushBaseAndCurrent" type cr
        else
            s" [FAIL]: addrStackGet/PushBaseAndCurrent 1" type cr
        then
    else
        s" [FAIL]: addrStackGet/PushBaseAndCurrent 2" type cr
    then
    addrStackPutBaseAndCurrent \ ()
    789 1 addrStackOverwrite
    \ addr stack now contains 789 456
    addrStackPop \ 456
    addrStackPop \ 789
    789 = if
        456 = if
            s" [PASS]: addr stack pop" type cr
        else
            s" [FAIL]: addr stack pop 1" type cr
        then
    else
        s" [FAIL]: addr stack pop 2" type cr
    then
    321 addrStackPush
    654 addrStackPush
    987 addrStackPush
    1 addrStackSlide
    \ addrstack should now be 321 987
    addrStackPop \ 987 
    addrStackPop \ 987 321
    321 = if
        987 = if
            s" [PASS]: addr slide" type cr
        else
            s" [FAIL]: addr slide 1" type cr
        then
    else
            s" [FAIL]: addr slide 2" type cr
    then
    246 addrStackPush
    688 addrStackPush
    addrStackReInit
    711 addrStackPush
    addrStackDepth
    1 = if
        s" [PASS]: addrStackReInit" type cr
    else
        s" [FAIL]: addrStackReInit" type cr
    then
;

\ test 2 is various abnormal conditions
\ need to write this
: testAddrTest2
    s" [SKIP]: skipping addr stack abnormal conditions" type cr
;

: testAddrStackTotal
    initializeAddrStack
    123 addrStackPush
    456 addrStackPush
    addrStackReInit
    789 addrStackPush
    addrStackDepth
    1 = if
        addrStackTotalDepth
        3 = if
            0 addrStackTotalPeek
            789 = if
                1 addrStackTotalPeek
                456 = if
                    2 addrStackTotalPeek
                    123 = if
                        555 2 addrStackTotalOverwrite
                        2 addrStackTotalPeek
                        555 = if
                            s" [PASS]: addrstack TOTAL ops " type cr
                        else
                            s" [FAIL]: addrstack TOTAL 6: " type cr
                        then
                    else
                        s" [FAIL]: addrstack TOTAL 5: " type cr
                    then
                else
                    s" [FAIL]: addrstack TOTAL 1: " type cr
                then
            else
                s" [FAIL]: addrstack TOTAL 2: " type cr
            then
        else
            s" [FAIL]: addrstack TOTAL 3: " type cr
        then
    else
        s" [FAIL]: addrstack TOTAL 4: " type cr
    then
;

\
\  Dump stack tests
\
: testDumpStack1
    12 34 dumpStackPush
    56 78 dumpStackPush
    90 11 dumpStackPush
    dumpStackDepth
    3 = if
        s" [PASS]: dumpstack depth" type cr
    else
        s" [FAIL]: dumpstack depth" type cr
    then
    dumpStackPop  \ 90 11
    11 = if
        90 = if
            s" [PASS]: dumpstack pop" type cr
        else
            s" [FAIL]: dumpstack pop 1" type cr
        then
    else
        s" [FAIL]: dumpstack pop 2" type cr
    then
    0 dumpStackPeek \ 56 78
    78 = if
        56 = if
            s" [PASS]: dumpstack peek" type cr
        else
            s" [FAIL]: dumpstack peek 1" type cr
        then
    else
        s" [FAIL]: dumpstack peek 2" type cr
    then
;

\ testDumpStack2 is designed to test abnormal conditions
: testDumpStack2
    s" [SKIP]: skipping dump stack abnormal conditions" type cr
;

\
\  Heap tests. 
\
: testHeapAlloc
    5 0 NNum hAlloc \ heapAddr
    hBytesRemaining \ heapAddr bytesRemaining
    GMHEAPtotalsize 12 -
    = if \ heapAddr
        s" [PASS]: hBytesRemaining pass" type cr
    else
        s" [FAIL]: hBytesRemaining fail 1" type cr
    then
    dup \ heapAddr heapAddr
    hLookup \ heapAddr nodeLeft nodeRight nodeType
    NNum = if
        0= if
            5 = if
                s" [PASS]: hAlloc and hLookup pass" type cr
            else
                s" [FAIL]: hAlloc and hLookup fail 1" type cr
            then
        else
            s" [FAIL]: hAlloc and hLookup fail 2" type cr
        then
    else
        s" [FAIL]: hAlloc and hLookup fail 3" type cr
    then
    hPeek \ heapAddr nodeType
    NNum = if
        s" [PASS]: hPeek pass" type cr
    then
    dup \ heapAddr heapAddr
    10 7 rot \ heapAddr 10 7 heapAddr
    NAp swap \ heapAddr 10 7 NAp heapAddr
    hUpdate \ heapAddr
    hLookup \ nodeLeft nodeRight nodeType
    NAp = if
        7 = if
            10 = if
                s" [PASS]: hUpdate pass" type cr
            else
                s" [FAIL]: hUpdate fail 1" type cr
            then
        else
            s" [FAIL]: hUpdate fail 2" type cr
        then
    else
        s" [FAIL]: hUpdate fail 3" type cr
    then
;

: testConstructorHeapAlloc
    \ dup .
    100 101 102 3 12 NConstr hAlloc \ heapAddr
    hBytesRemaining \ heapAddr bytes
    GMHEAPtotalsize 28 -
    = if
        s" [PASS]: hAlloc constructor pass" type cr
    else
        s" [FAIL]: hAlloc constructor fail 1" type cr
    then
    \ heapAddr
    dup \ heapAddr heapAddr
    hLookup \ 100 101 102 3 12 NConstr
    NConstr = if
        12 = if
            3 = if
                102 = if
                    101 = if
                        100 = if
                            s" [PASS]: hLookup constructor success" type cr
                        else
                            s" [FAIL]: hLookup constructor fail 1" type cr
                        then
                    else
                        s" [FAIL]: hLookup constructor fail 2" type cr
                    then
                else
                    s" [FAIL]: hLookup constructor fail 3" type cr
                then
            else
                s" [FAIL]: hLookup constructor fail 4" type cr
            then
        else
            s" [FAIL]: hLookup constructor fail 5" type cr
        then
    else
        s" [FAIL]: hLookup constructor fail 6" type cr
    then
    \ heapAddr
    >r
    200 201 2 15 NConstr r@ hUpdate
    r>
    hLookup \ 200 201 2 15 NConstr
    NConstr = if
        15 = if
            2 = if
                201 = if
                    200 = if
                        s" [PASS]: hLookup2 constructor success" type cr
                    else
                        s" [FAIL]: hLookup2 constructor fail 1" type cr
                    then
                else
                    s" [FAIL]: hLookup2 constructor fail 2" type cr
                then
            else
                s" [FAIL]: hLookup2 constructor fail 3" type cr
            then
        else
            s" [FAIL]: hLookup2 constructor fail 4" type cr
        then
    else
        s" [FAIL]: hLookup2 constructor fail 5" type cr
    then
;

: testHeapGlobalAlloc
    Add Mul Div 3 hGlobalAlloc \ heapAddr
    4 +
    absoluteGlobalHeapAddress
    setInstructionPointer
    getNextInstruction
    Add = if
        getNextInstruction
        Mul = if
            getNextInstruction
            Div = if
                s" [PASS]: hGlobalAlloc pass" type cr
                s" [PASS]: hGlobalLookup pass" type cr
            else
                s" [FAIL]: testHeapGlobalAlloc fail 1" type cr
            then
        else
            s" [FAIL]: testHeapGlobalAlloc fail 2" type cr
        then
    else
        s" [FAIL]: testHeapGlobalAlloc fail 3" type cr
    then
;

: testHeapGarbageCollection
    \ Test setForwardBit
    5 0 NNum hAlloc  \ heapAddr5
    drop
    10 0 NNum hAlloc \ heapAddr10
    15 0 NNum hAlloc \ heapAddr10 heapAddr15
    dup              \ heapAddr10 heapAddr15 heapAddr15
    rot              \ heapAddr15 heapAddr15 heapAddr10
    swap             \ heapAddr15 newheapAddr10 oldheapAddr15
    setForwardBit    \ oldheapAddr15 newheapAddr10
    swap             \ newHeapAddr10 oldheapAddr15 
    hLookup          \ newheapAddr10 nodeLeft nodeRight nodeType
    isForwardBitSet
    if               \ newheapAddr10 nodeLeft nodeRight
        drop         \ newheapAddr10 nodeLeft
        = if
            s" [PASS]: heap test: setForwardBit" type cr
        else
            s" [FAIL]: heap test: setForwardBit fail 1" type cr
        then
    else
        s" [FAIL]: setForwardBit fail 2" type cr
    then
    \ Test hCopyToInactive
    20 0 NNum hCopyToInactive \ heapAddr
    drop \ ()
    GMHEAPnextunusedinactive @
    12 = if
        s" [PASS]: heap test: hCopyToInactive" type cr
    else
        s" [FAIL]: heap test: hCopyToInactive fail 1" type cr
    then
    \ Test hEvacuateNode
    25 0 NNum hAlloc \ heapAddr
    dup              \ heapAddr heapAddr
    hEvacuateNode    \ oldHeapAddr newHeapAddr
    swap             \ newHeapAddr oldHeapAddr
    hLookup          \ newHeapAddr nodeLeft nodeRight nodeType
    isForwardBitSet
    if               \ newHeapAddr nodeLeft nodeRight
        drop         \ newHeapAddr nodeLeft
        = if
            s" [PASS]: heap test: hEvacuateNode" type cr
        else
            s" [FAIL]: heap test: hEvacuateNode fail 1" type cr
        then
    else
        s" [FAIL]: heap test: hEvacuateNode fail 2" type cr
    then
    \ Test hScavenge first with just an NAp node
    initializeHeap
    10 0 NNum hAlloc \ heapAddr10
    dup              \ heapAddr10 heapAddr10
    15 0 NNum hAlloc \ heapAddr10 heapAddr10 heapAddr15
    dup              \ heapAddr10 heapAddr10 heapAddr15 heapAddr15
    rot              \ heapAddr10 heapAddr15 heapAddr15 heapAddr10
    swap             \ heapAddr10 heapAddr15 heapAddr10 heapAddr15
    NAp hAlloc       \ heapAddr10 heapAddr15 heapAddrAp
    dup              \ heapAddr10 heapAddr15 heapAddrAp heapAddrAp
    hEvacuateNode    \ heapAddr10 heapAddr15 heapAddrAp newheapAddrAp
    drop             \ heapAddr10 heapAddr15 heapAddrAp
    \ now, we should have the NAp in the new heap.  The 10 and 15
    \ should be brought over by the hScavenge call.
    GMHEAPnextunusedinactive @ 12 /
    1 = if
        hScavenge
        GMHEAPnextunusedinactive @ 12 /
        3 = if
            \ after the scavenge, there are properly 3 elements in the
            \ inactive heap.
            hLookup
            \ heapAddr10 heapAddr15 ApnodeLeft ApnodeRight ApnodeType
            isForwardBitSet
            \ heapAddr10 heapAddr15 ApnodeLeft ApnodeRight bool
            if
                2drop   \ heapAddr10 heapAddr15
                hLookup \ heapAddr10 15nodeLeft 15nodeRight 15nodeType
                2drop   \ heapAddr10 15nodeLeft
                hLookup
                \ heapAddr10 new15nodeLeft new15nodeRight new15nodeType
                NNum = if
                    drop
                    15 = if
                        \ heapAddr10
                        hLookup \ 10nodeLeft 10nodeRight 10nodeType
                        2drop   \ 10nodeLeft
                        hLookup \ newnodeLeft10 newNodeRight10 newNodeType10
                        2drop
                        10 = if
                            s" [PASS]: heap test: hScavenge pass" type cr
                        else
                            s" [FAIL]: heap test: hScavenge fail 1d" type cr
                        then
                    else
                        s" [FAIL]: heap test: hScavenge fail 1c" type cr
                    then
                else
                    s" [FAIL]: heap test: hScavenge fail 1b" type cr
                then
            else
                s" [FAIL]: heap test: hScavenge fail 1a" type cr
            then
        else
            s" [FAIL]: heap test: hScavenge fail 2" type cr
        then
    else
        s" [FAIL]: heap test: hScavenge fail 3" type cr
    then
    \ Test heap switching
    hSwitchHeaps
    hBytesRemaining
    GMHEAPtotalsize 12 -
    = if
        s" [PASS]: heap test: switching heaps" type cr
    else
        s" [FAIL]: heap test: switching heaps fail 1" type cr
    then
    30 0 NNum hAlloc \ 30heapAddr
    dup              \ 30heapAddr 30heapAddr
    GMHEAPtotalsize
    \ Make sure when the heap was switched that the heap address was
    \ greater than the total size of the first heap.  This assures the
    \ switch was actually successful.
    > if
        s" [PASS]: heap test: switching heaps part 1a" type cr
    else
        s" [FAIL]: heap test: switching heaps fail 1a" type cr
    then
    dup              \ 30heapAddr 30heapAddr
    0 NInd hAlloc    \ 30heapAddr heapAddrInd
    hEvacuateNode    \ 30heapAddr newheapAddrInd
    hScavenge
    hSwitchHeaps
    hLookup          \ 30heapAddr indNodeLeft indNodeRight indNodeType
    isForwardBitSet
    if               \ 30heapAddr indNodeLeft indNodeRight
        s" [FAIL]: heap test: switching heaps fail 2" type cr
    else
        drop  \ 30heapAddr indNodeLeft
        hLookup
        \ 30heapAddr 30newheapNodeLeft 30newheapNodeRight 30newheapNodeType
        isForwardBitSet
        if      \ 30heapAddr 30newheapNodeLeft 30newheapNodeRight
            s" [FAIL]: heap test: switching heaps fail 3" type cr
        else
            drop   \ 30heapAddr 30newheapNodeLeft 
            30 = if  \ 30heapAddr
                hLookup
                \ 30heapoldNodeLeft 30heapoldNodeRight 30heapoldNodeType
                isForwardBitSet
                if  \ 30heapoldNodeLeft 30heapoldNodeRight
                    2drop
                    s" [PASS]: heap test: switching heaps part 2" type cr
                else
                    s" [FAIL]: heap test: switching heaps fail 4" type cr
                then
            else
                s" [FAIL]: heap test: switching heaps fail 5" type cr
            then
        then
    then
;

: testHeapGarbageCollection2
    initializeHeap
    initializeAddrStack
    7 0 NNum hAlloc \ 7heapAddr
    9 0 NNum hAlloc \ 9heapAddr
    2dup  \ 7heapAddr 9heapAddr 7heapAddr 9heapAddr
    addrStackPush
    addrStackPush
    swap  \ 9heapAddr 7heapAddr
     \ addr stack contains: 9heapAddr 7heapAddr
    hGarbageCollect
    \ Note the garbage collection should modify the address stack.
    addrStackPop \ 9heapAddr 7heapAddr 7newheapAddr
    dup   \ 9heapAddr 7heapAddr 7newheapAddr 7newheapAddr
    rot   \ 9heapAddr 7newheapAddr 7newheapAddr 7heapAddr
    <> if   \ 9heapAddr 7newheapAddr 
        hLookup
        \ 9heapAddr 7newHeapNodeLeft 7newHeapNodeRight 7newHeapNodeType
        2drop   \ 9heapAddr 7newHeapNodeLeft
        7 = if  \ 9heapAddr
            addrStackPop  \ 9heapAddr 9newheapAddr
            dup           \ 9heapAddr 9newheapAddr 9newheapAddr
            rot           \ 9newheapAddr 9newheapAddr 9heapAddr
            <> if  \ 9newheapAddr
                hLookup
                \ 9newHeapNodeLeft 9newHeapNodeRight 9newHeapNodeType
                2drop   \ 9newHeapNodeLeft
                9 = if 
                    s" [PASS]: heap test: collection" type cr
                else
                    s" [FAIL]: heap test: collection 1" type cr
                then
            else
                s" [FAIL]: heap test: collection 2" type cr
            then
        else
            s" [FAIL]: heap test: collection 3" type cr
        then
    else
        s" [FAIL]: heap test: collection 4" type cr
    then
;

\ Useful for testHeapMaxOut function
: testHeapHeapDropUtility
    hAlloc
    drop
;

: testHeapMaxOut
    1
    1
    do
        i i NNum ['] testHeapHeapDropUtility catch
        throwHeapFull = if
            s" [PASS]: HEAP full on try: " type i . cr
            leave
        then
    loop
;

\
\  Test evaluation system
\
: testEval1
    0
    Pushint 3
    UpdateI 0
    Pop 0
    Unwind
    7
    hGlobalAlloc
    NGlobal
    hAlloc
    1
    aInsert

    Pushglobal 1
    EvalI
    EndOfCode
    4 injectInstructions
    eval
    
    3 = if
        s" [PASS]: evaltest1 (Simple Number)" type cr
    else
        s" [FAIL]: evaltest1 1 (Simple Number)" type cr
    then
    drop
;

: testEval2
    1
    Push 0
    UpdateI 1
    Pop 1
    Unwind
    7
    hGlobalAlloc
    NGlobal
    hAlloc
    13
    aInsert

    0
    Pushint 3
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
    4 injectInstructions
    eval
    
    3 = if
        16 = if
            s" [PASS]: evaltest2 (I combinator)" type cr
        else
            s" [FAIL]: evaltest2 1 (I combinator)" type cr
        then
    else
            s" [FAIL]: evaltest2 2 (I combinator)" type cr
    then
;

: testEval3
    2
    Push 0
    UpdateI 2
    Pop 2
    Unwind
    7
    hGlobalAlloc
    NGlobal
    hAlloc
    13
    aInsert

    3
    Push 2
    Push 2
    Mkap
    Push 3
    Push 2
    Mkap
    Mkap
    UpdateI 3
    Pop 3
    Unwind
    16
    hGlobalAlloc
    NGlobal
    hAlloc
    14
    aInsert

    0
    Pushglobal 13
    Pushglobal 13
    Pushglobal 14
    Mkap
    Mkap
    UpdateI 0
    Pop 0
    Unwind
    13
    hGlobalAlloc
    NGlobal
    hAlloc
    15
    aInsert

    0
    Pushint 14
    Pushglobal 15
    Mkap
    UpdateI 0
    Pop 0
    Unwind
    10
    hGlobalAlloc
    NGlobal
    hAlloc
    16
    aInsert

    Pushglobal 16
    EvalI
    EndOfCode
    4 injectInstructions
    eval
    
    14 = if
        s" [PASS]: evaltest3 (S K K = I)" type cr
    else
        s" [FAIL]: evaltest3 1 (S K K = I)" type cr
    then
    
    drop
;

: testEval4
    2
    Push 0
    UpdateI 2
    Pop 2
    Unwind
    7
    hGlobalAlloc
    NGlobal
    hAlloc
    13
    aInsert

    3
    Push 2
    Push 2
    Mkap
    Push 3
    Push 2
    Mkap
    Mkap
    UpdateI 3
    Pop 3
    Unwind
    16
    hGlobalAlloc
    NGlobal
    hAlloc
    14
    aInsert

    3
    Push 2
    Push 2
    Mkap
    Push 1
    Mkap
    UpdateI 3
    Pop 3
    Unwind
    13
    hGlobalAlloc
    NGlobal
    hAlloc
    15
    aInsert

    1
    Push 0
    Push 1
    Pushglobal 15
    Mkap
    Mkap
    UpdateI 1
    Pop 1
    Unwind
    13
    hGlobalAlloc
    NGlobal
    hAlloc
    16
    aInsert

    0
    Pushglobal 13
    Pushglobal 13
    Pushglobal 14
    Mkap
    Mkap
    UpdateI 0
    Pop 0
    Unwind
    13
    hGlobalAlloc
    NGlobal
    hAlloc
    17
    aInsert

    0
    Pushint 7
    Pushglobal 17
    Pushglobal 16
    Pushglobal 16
    Pushglobal 16
    Mkap
    Mkap
    Mkap
    Mkap
    UpdateI 0
    Pop 0
    Unwind
    19
    hGlobalAlloc
    NGlobal
    hAlloc
    18
    aInsert

    Pushglobal 18
    EvalI
    EndOfCode
    4 injectInstructions
    eval
    
    7 = if
        s" [PASS]: evaltest4 (twice twice twice id 7)" type cr
    else
        s" [FAIL]: evaltest4 1 (twice twice twice id 7)" type cr
    then
    
    drop
;

: testEval5
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
    Pushint 5
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
    4 injectInstructions
    eval
    
    5 = if
        s" [PASS]: evaltest5 (fib 5)" type cr
    else
        s" [FAIL]: evaltest5 1 (fib 5)" type cr
    then
    drop
;

: testEval6
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
    Alloc 1
    Push 0
    Push 2
    Mkap
    UpdateI 0
    Push 0
    Slide 1
    UpdateI 1
    Pop 1
    Unwind
    18
    hGlobalAlloc
    NGlobal
    hAlloc
    13
    aInsert

    2
    Pushint 2
    Push 2
    Pushglobal 2
    Mkap
    Mkap
    Push 1
    Mkap
    Pushint 1
    Push 3
    Pushglobal 2
    Mkap
    Mkap
    Push 2
    Mkap
    Pushglobal 1
    Mkap
    Mkap
    Pushint 1
    Pushint 2
    Push 4
    Pushglobal 9
    Mkap
    Mkap
    Pushglobal 12
    Mkap
    Mkap
    Mkap
    UpdateI 2
    Pop 2
    Unwind
    46
    hGlobalAlloc
    NGlobal
    hAlloc
    14
    aInsert

    0
    Pushglobal 14
    Pushglobal 13
    Mkap
    UpdateI 0
    Pop 0
    Unwind
    10
    hGlobalAlloc
    NGlobal
    hAlloc
    15
    aInsert

    0
    Pushint 6
    Pushglobal 15
    Mkap
    UpdateI 0
    Pop 0
    Unwind
    10
    hGlobalAlloc
    NGlobal
    hAlloc
    16
    aInsert

    Pushglobal 16
    EvalI
    EndOfCode
    4 injectInstructions
    eval
    
    8 = if
        s" [PASS]: evaltest6 (fib 6) using Y" type cr
    else
        s" [FAIL]: evaltest6 1 (fib 6) using Y" type cr
    then
    drop
 
;

: testEval7
    2
    Push 0
    UpdateI 2
    Pop 2
    Unwind
    7
    hGlobalAlloc
    NGlobal
    hAlloc
    13
    aInsert

    2
    Push 1
    UpdateI 2
    Pop 2
    Unwind
    7
    hGlobalAlloc
    NGlobal
    hAlloc
    14
    aInsert

    4
    Push 1
    Push 1
    Push 4
    Mkap
    Mkap
    UpdateI 4
    Pop 4
    Unwind
    13
    hGlobalAlloc
    NGlobal
    hAlloc
    15
    aInsert

    2
    Push 1
    UpdateI 2
    Pop 2
    Unwind
    7
    hGlobalAlloc
    NGlobal
    hAlloc
    16
    aInsert

    1
    Pushglobal 19
    Pushglobal 13
    Push 2
    Mkap
    Mkap
    UpdateI 1
    Pop 1
    Unwind
    13
    hGlobalAlloc
    NGlobal
    hAlloc
    17
    aInsert

    1
    Pushglobal 19
    Pushglobal 14
    Push 2
    Mkap
    Mkap
    UpdateI 1
    Pop 1
    Unwind
    13
    hGlobalAlloc
    NGlobal
    hAlloc
    18
    aInsert

    0
    Pushglobal 19
    UpdateI 0
    Pop 0
    Unwind
    7
    hGlobalAlloc
    NGlobal
    hAlloc
    19
    aInsert

    1
    Alloc 1
    Push 0
    Push 2
    Pushglobal 15
    Mkap
    Mkap
    UpdateI 0
    Push 0
    Slide 1
    UpdateI 1
    Pop 1
    Unwind
    21
    hGlobalAlloc
    NGlobal
    hAlloc
    20
    aInsert

    0
    Pushint 4
    Pushglobal 20
    Mkap
    Pushglobal 18
    Mkap
    Pushglobal 18
    Mkap
    Pushglobal 17
    Mkap
    UpdateI 0
    Pop 0
    Unwind
    19
    hGlobalAlloc
    NGlobal
    hAlloc
    21
    aInsert

    Pushglobal 21
    EvalI
    EndOfCode
    4 injectInstructions
    eval
    
    4 = if
        s" [PASS]: evaltest7 ( hd (tl (tl (tl (infinite 4))) ) with letrec" type cr
    else
        s" [FAIL]: evaltest7 1 ( hd (tl (tl (tl (infinite 4))) ) with letrec" type cr
    then
    drop
;

\ Test simple If FALSE condition
: testEval8
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

    0
    Pushint 300
    Pushint 100
    Pushint 3
    Pushint 4
    Pushglobal 9
    Mkap
    Mkap
    Pushglobal 12
    Mkap
    Mkap
    Mkap
    UpdateI 0
    Pop 0
    Unwind
    22
    hGlobalAlloc
    NGlobal
    hAlloc
    13
    aInsert

    Pushglobal 13
    EvalI
    EndOfCode
    4 injectInstructions
    eval
    
    300 = if
        s" [PASS]: evaltest simple if FALSE condition" type cr
    else
        s" [FAIL]: evaltest simple if FALSE condition" type cr
    then
    drop
;

\ Test simple If TRUE condition
: testEval9
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

    0
    Pushint 300
    Pushint 100
    Pushint 4
    Pushint 3
    Pushglobal 9
    Mkap
    Mkap
    Pushglobal 12
    Mkap
    Mkap
    Mkap
    UpdateI 0
    Pop 0
    Unwind
    22
    hGlobalAlloc
    NGlobal
    hAlloc
    13
    aInsert

    Pushglobal 13
    EvalI
    EndOfCode
    4 injectInstructions
    eval
    
    100 = if
        s" [PASS]: evaltest simple if TRUE condition" type cr
    else
        s" [FAIL]: evaltest simple if TRUE condition" type cr
    then
    drop
;

: dumpInitialStats
    s" [begin stats: " type cr
    s" GMGLOBALS: " type
    GMGLOBALS . cr
    s" GMGLOBALSnextunused: " type
    GMGLOBALSnextunused @ . cr
    s" GMHEAP: " type 
    GMHEAP . cr
    s" GMHEAPnextunused: " type 
    GMHEAPnextunused @ . cr
    s" ADDRSTACKbase: " type 
    ADDRSTACKbase . cr
    s" ADDRSTACKcurrentbase: " type 
    ADDRSTACKcurrentbase @ . cr
    s" ADDRSTACKcurrenttop: " type 
    ADDRSTACKcurrenttop @ . cr
    s" DUMPSTACK: " type 
    DUMPSTACK . cr
    s" DUMPSTACKcurrenttop: " type 
    DUMPSTACKcurrenttop @ . cr
    s"  end stats]" type cr cr
;

: testEvalSystem
    initializeHeap
    initializeGlobals
    initializeAddrStack
    initializeDumpStack
    initializeInstrStack
    \ dumpInitialStats
    testEval1
    
    initializeHeap
    initializeGlobals
    initializeAddrStack
    initializeDumpStack
    initializeInstrStack
\     \ dumpInitialStats
    testEval2
    
    initializeHeap
    initializeGlobals
    initializeAddrStack
    initializeDumpStack
    initializeInstrStack
\     \ dumpInitialStats
    testEval3
    
    initializeHeap
    initializeGlobals
    initializeAddrStack
    initializeDumpStack
    initializeInstrStack
\     \ dumpInitialStats
    testEval4

    initializeHeap
    initializeGlobals
    initializeAddrStack
    initializeDumpStack
    initializeInstrStack
\     \ dumpInitialStats
    testEval9
    
    initializeHeap
    initializeGlobals
    initializeAddrStack
    initializeDumpStack
    initializeInstrStack
\     \ dumpInitialStats
    testEval8

    initializeHeap
    initializeGlobals
    initializeAddrStack
    initializeDumpStack
    initializeInstrStack
\     \ dumpInitialStats
    testEval5

    initializeHeap
    initializeGlobals
    initializeAddrStack
    initializeDumpStack
    initializeInstrStack
\     \ dumpInitialStats
    testEval6

    initializeHeap
    initializeGlobals
    initializeAddrStack
    initializeDumpStack
    initializeInstrStack
\     \ dumpInitialStats
    testEval7
;

: testDumpStack
    initializeDumpStack
    testDumpStack1
    testDumpStack2
;

: testAddrStack
    initializeAddrStack
    testAddrTest1
    testAddrTest2
    testAddrStackTotal
;

\ Perform all tests when catch/throw work
: testGlobals
\     initializeGlobals
\     testGlobalsMaxOut
    initializeGlobals
    testGlobalSimpleCase
\    testGlobalNotPresent
;

\ Perform all tests when catch/throw work
:  testHeap
    \ initializeHeap
    \ testHeapAlloc
    initializeHeap
    testConstructorHeapAlloc
    \ testHeapGlobalAlloc
    \ initializeHeap
    \ testHeapGarbageCollection
    \ testHeapGarbageCollection2

\    initializeHeap
\    testHeapMaxOut
;

: delay
    10000
    0
    do
        65 drop
    loop
;


: runTests
    cr
    100
    0
    do
        s" Run:" type
        i . cr
        testHeap
        hBytesRemaining drop
        \ testGlobals
        \ testAddrStack
        \ testDumpStack
        \ testEvalSystem
        cr
    loop
;

: foo
    cr
    100
    0
    do
        s" Run:" type
        i . cr
        testHeap
        hBytesRemaining drop
        \ testGlobals
        \ testAddrStack
        \ testDumpStack
        \ testEvalSystem
        cr
    loop
;
