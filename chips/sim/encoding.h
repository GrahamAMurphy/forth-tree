#define STATECODE \
{int, \
exec, \
load, \
store, \
squash, \
rover1, \
rover2, \
runder1, \
runder2, \
pover1, \
pover2, \
punder1, \
punder2} \
{0000, \
0001, \
0010, \
0011, \
0100, \
1000, \
1001, \
1010, \
1011, \
1100, \
1101, \
1110, \
1111}
#define rombase ^b0001
#define iobase ^b0010
#define rsp ^b00000
#define psp ^b00001
#define scc ^b00010
#define mask ^b00100
#define mode ^b00101
#define edge ^b00110
#define levl ^b00111
#define attrfirst ^b10000
#define attrrange ^b1----
#define TYPECODE \
{flow, \
micro, \
lal, \
lah, \
loadcell, \
loadbyte, \
storecell, \
storebyte} \
{000, \
001, \
010, \
011, \
100, \
101, \
110, \
111}
#define FLOWCODE \
{call, \
does, \
branch, \
qbranch} \
{00, \
01, \
10, \
11}
#define RETCODE \
{noreturn, \
return} \
{0, \
1}
#define RCODE \
{s0, \
s1, \
s2, \
s3, \
r0, \
r1, \
r2, \
r3, \
u0, \
u1, \
u2, \
u3, \
pc, \
psw, \
zero, \
y} \
{0000, \
0001, \
0010, \
0011, \
0100, \
0101, \
0110, \
0111, \
1000, \
1001, \
1010, \
1011, \
1100, \
1101, \
1110, \
1111}
#define PSCODE \
{nopps, \
popps, \
pushps} \
{00, \
01, \
10}
#define RSCODE \
{noprs, \
poprs, \
pushrs} \
{00, \
01, \
10}
#define CLASSCODE \
{alu, \
test, \
step, \
shift} \
{00, \
01, \
10, \
11}
#define ALUCONDCODE \
{clear, \
set, \
ovflow, \
vbar, \
greater, \
lesseq, \
neg, \
nbar, \
equal, \
nequal, \
ugreater, \
ulesseq, \
less, \
greatereq, \
cout, \
cbar} \
{1011, \
0000, \
1010, \
1100, \
0111, \
1000, \
0100, \
1110, \
0011, \
0001, \
1111, \
0010, \
1001, \
0110, \
0101, \
1101}
#define FLAGCODE \
{noflag, \
flag} \
{0, \
1}
#define CINCODE \
{czero, \
cone, \
cflag, \
cflagbar} \
{00, \
01, \
10, \
11}
#define ones ^b000011
#define zeros ^b001100
#define nopa ^b001010
#define nopb ^b001111
#define nota ^b000101
#define notb ^b000000
#define aandb ^b001110
#define abarandb ^b001101
#define aandbbar ^b001000
#define anandb ^b000001
#define aorb ^b001011
#define abarorb ^b000111
#define aorbbar ^b000010
#define anorb ^b000100
#define axorb ^b001001
#define anxorb ^b000110
#define aplusb ^b011001
#define aminusb ^b111001
#define bminusa ^b010110
#define negb ^b111111
#define incb ^b011111
#define decb ^b010000
#define bplusb ^b011100
#define flalucond ^b00
#define fly1 ^b01
#define fldivhelp ^b10
#define LEFTCODE \
{noleftshift, \
left} \
{0, \
1}
#define ynop ^b00
#define yleft ^b01
#define yright ^b10
#define STEPCODE \
{stepb, \
cadd, \
div, \
mul} \
{00, \
01, \
10, \
11}
#define SHIFTMODECODE \
{lsr, \
asr, \
lsl, \
rol, \
srflag, \
srflagbar, \
pri} \
{000, \
001, \
010, \
011, \
100, \
101, \
111}
#define IMMCODE \
{noimm, \
imm} \
{0, \
1}
