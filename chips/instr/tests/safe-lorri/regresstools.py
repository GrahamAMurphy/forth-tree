# (c) 2003 Johns Hopkins University / Applied Physics Laboratory
# by John R. Hayes
from __main__ import *
import Monitor
import Sequencer

# ------------------------------------------------------------------------
# Select instrument

# instr = 'MDS'; cmdprefix = instr; host = 'MESS'
# instr = 'LOR'; cmdprefix = instr; host = 'NH'
# instr = 'PEP'; cmdprefix = instr; host = 'NH'
# instr = 'VSL'; cmdprefix = instr; host = 'NH'
instr = 'ATAS'; cmdprefix = instr; host = 'NH'
# instr = 'CRM'; cmdprefix = instr; host = 'MRO'

# Select memory test page
tstpg = 6

# ------------------------------------------------------------------------
# MESSENGER-specific things here

if host == 'MESS':
  from core.core import CMD
  from common.MET import SetMET

  dOpcodes = {
	'CMD_NULL' : 0x02,
	'CMD_WRAP' : 0x03,
	'MAC_CHECK' : 0x11,
	'MAC_DEF' : 0x10,
	'MAC_DELAY' : 0x11,
	'MAC_END' : 0x12,
	'MAC_ENDDEF' : 0x13,
	'MAC_HALT' : 0x14,
	'MAC_LOOP_BEGIN' : 0x18,
	'MAC_LOOP_END' : 0x19,
	'MAC_NEST' : 0x15,
	'MAC_PAUSE' : 0x16,
	'MAC_READ' : 0x25,
	'MAC_RESTORE' : 0x1c,
	'MAC_RUN' : 0x17,
	'MAC_RUN_SILENT' : 0x1f,
	'MAC_SAVE' : 0x1b,
	'MEM_CHECK' : 0x04,
	'MEM_COPY' : 0x05,
	'MEM_LOAD' : 0x06,
	'MEM_READ' : 0x07,
	'MEM_READ_ABRT' : 0x08,
	'MEM_READ_INT' : 0x24,
	'MEM_RUN' : 0x09,
	'MEM_STR_LOAD' : 0x0a,
	'MEM_STR_READ' : 0x0b,
	'MEM_WR_EN' : 0x23,
	'MON_CNTRL' : 0x0c,
	'PWR_OFF' : 0x20,
	'ROM_BOOT' : 0x0e,
	'ROM_GO' : 0x0f,
	'SAFE' : 0x1d,
	'SHUTDOWN' : 0x1e,
	'SOFT_RESET' : 0x22,
	'STAT_CLR' : 0x01,		# named COUNTER_CLR on MESSENGER
	'STAT_INT' : 0x0d,
	'STAT_MEM' : 0x21
  }

  dTrueNames = {
	'STAT_CLR' : 'COUNTER_CLR'
  }

  def fSetTime(t):
    SetMET(t)

  def fPktSeqNums(block):
    return (block.SeqCount, block.MET)

  def fPktSeqNext(seq, time, interval):
    return (seq+1, time+interval)

  def fPktDataLen(blk):
    return blk.Length + 1 - 4

  def fLoadFile(path, addr):
    core.loadFile.fLOAD_FILE(path, 'MDS', addr)

# ------------------------------------------------------------------------
# LORRI/PEPSSI-specific things here

if host == 'NH':
  from common.core import CMD
  from sceAPI import SetMET

  dOpcodes = {
	'CMD_NULL' : 0x01,
	'CMD_WRAP' : 0x02,
	'MAC_CHECK' : 0x03,
	'MAC_DEF' : 0x04,
	'MAC_DELAY' : 0x05,
	'MAC_END' : 0x06,
	'MAC_ENDDEF' : 0x07,
	'MAC_HALT' : 0x08,
	'MAC_LOOP_BEGIN' : 0x09,
	'MAC_LOOP_END' : 0x0a,
	'MAC_NEST' : 0x0b,
	'MAC_PAUSE' : 0x0c,
	'MAC_READ' : 0x0d,
	'MAC_RESTORE' : 0x0e,
	'MAC_RUN' : 0x0f,
	'MAC_RUN_SILENT' : 0x10,
	'MAC_SAVE' : 0x11,
	'MEM_CHECK' : 0x12,
	'MEM_COPY' : 0x13,
	'MEM_LOAD' : 0x14,
	'MEM_READ' : 0x15,
	'MEM_READ_ABRT' : 0x16,
	'MEM_RUN' : 0x17,
	'MEM_STR_LOAD' : 0x18,
	'MEM_STR_READ' : 0x19,
	'MEM_WR_EN' : 0x1a,
	'MON_CNTRL' : 0x1b,
	'PWR_OFF' : 0x1c,
	'ROM_BOOT' : 0x23,
	'ROM_GO' : 0x24,
	'SAFE' : 0x1d,
	'SHUTDOWN' : 0x1e,
	'SOFT_RESET' : 0x1f,
	'STAT_CLR' : 0x20,
	'STAT_INT' : 0x21,
	'STAT_MEM' : 0x22
  }

  dTrueNames = {
  # none ...
  }

  def fSetTime(t):
    SetMET(t)

  def fPktSeqNums(block):
    return (block.SeqCount, block.MET)

  def fPktSeqNext(seq, time, interval):
    return (seq+1, time+interval)

  def fPktDataLen(blk):
    return blk.Length + 1 - 4

  if instr == 'LOR':
    fLoadFile = i_LORRI.lor_com_cmds.fLOR_LOAD_FILE
  elif instr == 'VSL':
    fLoadFile = i_VSLIT.vsl_com_cmds.fVSL_LOAD_FILE
  elif instr == 'ATAS':
    fLoadFile = i_ATAS.atas_com_cmds.fATAS_LOAD_FILE
  else:
    fLoadFile = i_PEPSSI.pep_com_cmds.fPEP_LOAD_FILE

# ------------------------------------------------------------------------
# CRISM-specific things here

if host == 'MRO':
  from core.core import CMD
  from core.CRMTiming import fSetTime

  dOpcodes = {
	'CMD_NULL' : 0x01,
	'CMD_WRAP' : 0x02,
	'MAC_CHECK' : 0x03,
	'MAC_DEF' : 0x04,
	'MAC_DELAY' : 0x05,
	'MAC_END' : 0x06,
	'MAC_ENDDEF' : 0x07,
	'MAC_HALT' : 0x08,
	'MAC_LOOP_BEGIN' : 0x09,		# MAC_LOOP_BEG on CRISM
	'MAC_LOOP_END' : 0x0a,
	'MAC_NEST' : 0x0b,
	'MAC_PAUSE' : 0x0c,
	'MAC_READ' : 0x0d,
	'MAC_RESTORE' : 0x0e,
	'MAC_RUN' : 0x0f,
	'MAC_RUN_SILENT' : 0x10,		# MAC_RUN_SIL on CRISM
	'MAC_SAVE' : 0x11,
	'MEM_CHECK' : 0x12,
	'MEM_COPY' : 0x13,
	'MEM_LOAD' : 0x14,
	'MEM_READ' : 0x15,
	'MEM_READ_ABRT' : 0x16,
	'MEM_RUN' : 0x17,
	'MEM_STR_LOAD' : 0x18,
	'MEM_STR_READ' : 0x19,
	'MEM_WR_EN' : 0x1a,
	'MON_CNTRL' : 0x1b,
	'ROM_BOOT' : 0x1e,
	'ROM_GO' : 0x1f,
	'STAT_CLR' : 0x1c,
	'STAT_INT' : 0x1d,
	'STAT_MEM' : 0x20,
  }

  dTrueNames = {
	'MAC_LOOP_BEGIN' : 'MAC_LOOP_BEG',
	'MAC_RUN_SILENT' : 'MAC_RUN_SIL'
  }

  def fPktSeqNums(block):
    return (0, block.Time_Tag)

  def fPktSeqNext(seq, time, interval):
    return (0, time+interval)

  def fPktDataLen(blk):
    return blk.Length + 1

  fLoadFile = core.loadFile.fLOAD_FILE

# ------------------------------------------------------------------------
# Useful helpers

# Blocks

bStatus     = eval(instr + '_Status')
bEcho       = eval(instr + '_CmdEcho')
bMemCheck   = eval(instr + '_MemChecksum')
bMemRead    = eval(instr + '_MemDump')
bMacCheck   = eval(instr + '_MacroChecksum')
bMacRead    = eval(instr + '_MacroDump')
bBootStatus = eval(instr + '_Boot_Status')

# Error/status logging

ierrors = 0

def C(s):
  print s

def ResetError():
  global ierrors
  ierrors = 0

def Error(s):
  global ierrors
  ierrors += 1
  print s

def ErrorReport():
  if ierrors == 0:
    print "No errors"
  else:
    print "%d errors seen" % ierrors

# Commanding

def fSendCmd(name, *args):
  CMD(cmdprefix + '_' + dTrueNames.get(name, name), args)

# Packet checking

def fPacketSeen(oSeq, block, timeout):
  seen = 1
  try:
    oSeq.Wait(block, lambda:1, (), timeout)
  except Sequencer.TimeoutError:
    seen = 0
  return seen

def fCheckNoPacket(oSeq, block):
  if fPacketSeen(oSeq, block, 10):
      Error('Unexpected packet seen')

def fCheckPacket(oSeq, block, f):
  if fPacketSeen(oSeq, block, 10):
    if not f():
      Error('Bad packet data')
  else:
    Error('No packet')

def fCheckSeq(oSeq, block, nblks, interval):
  def fVerifySeq(block, seq, time):
    # print seq, time, fPktSeqNums(block)
    if not (seq,time)==fPktSeqNums(block):
      Error('Packet sequence error')
    return 1				# alway return true
  try:
    oSeq.Wait(block, lambda:1, (), interval+5)
    seq,time = fPktSeqNums(block)
    for i in range(nblks):		# check expected seq. numbers and times
      seq,time = fPktSeqNext(seq, time, interval)
      oSeq.Wait(block, fVerifySeq, (block, seq, time), interval+5)
  except Sequencer.TimeoutError:
    Error('Packet sequence timeout')

def fCheckEcho(oSeq, cmdname, result, args, timeout=10):
  if fPacketSeen(oSeq, bEcho, timeout):
    if not((bEcho.Opcode==dOpcodes[cmdname]) and (bEcho.Result==result) and
            (fPktDataLen(bEcho)-2==len(args))):
      Error('Bad command echo')
    else:
      argsok = 1
      for i in range(len(args)):
        if bEcho.Args[i]<>args[i]:
          argsok = 0
      if not argsok:
        Error('Bad command echo args')
  else:
    Error('No command echo')

def fCheckMemCheck(oSeq, src, words, sum):
  if fPacketSeen(oSeq, bMemCheck, 10):
    if not ((bMemCheck.Address==src) and
		(bMemCheck.MemLength==words) and
		(bMemCheck.CRC_Value==sum)):
      Error('Bad memory checksum')
  else:
    Error('No memory checksum')

def fCheckMemRead(oSeq, lowaddr, highaddr, bytes, expdata):
  if fPacketSeen(oSeq, bMemRead, 10):
    if not ((bMemRead.Address>=lowaddr) and
		(bMemRead.Address<=highaddr) and
		(bMemRead.DumpLength==bytes)):
      Error('Bad memory read')
    else:
      dataok = 1
      for i in range(len(expdata)):
        if bMemRead.DumpData[i]<>expdata[i]:
          dataok = 0
      if not dataok:
        Error('Bad memory read data')
  else:
    Error('No memory read')

def fCheckMacCheck(oSeq, firstid, lastid, sums):
  if fPacketSeen(oSeq, bMacCheck, 10):
    if not ((bMacCheck.FirstMacro==firstid) and
		(bMacCheck.LastMacro==lastid) and
		(fPktDataLen(bMacCheck)-2==2*len(sums))):
      Error('Bad macro checksum packet')
    else:
      dataok = 1
      for i in range(len(sums)):
        if bMacCheck.Checksums[i]<>sums[i]:
          dataok = 0
      if not dataok:
        Error('Bad macro checksum')
  else:
    Error('No memory checksum')

def fCheckMacRead(oSeq, macid, blockno, expdata):
  if fPacketSeen(oSeq, bMacRead, 10):
    if not ((bMacRead.MacroId==macid) and
		(bMacRead.BlockIndex==blockno)):
      Error('Bad macro read')
    else:
      dataok = 1
      for i in range(len(expdata)):
        if bMacRead.MacroData[i]<>expdata[i]:
          dataok = 0
      for i in range(len(expdata), 32):
        if bMacRead.MacroData[i]<>0:
          dataok = 0
      if not dataok:
        Error('Bad macro read data')
  else:
    Error('No macro read')

def fCRC(b):
  crc = 0xffff
  for i in range(len(b)):
    crc ^= b[i] << 8
    for i in range(8):
      if crc & 0x8000:
        crc = crc<<1 ^ 0x1021
      else:
        crc = crc<<1
  return crc&0xffff

def fMacChecksum(b):
  return fCRC(b + [0]*((32 - len(b))%32))

# ------------------------------------------------------------------------
# Command echo sequence verification (from M. Paul)

iEchoSeqEnb = 0
lEchoSeqData = []

def fStartEchoSeq():
  global iEchoSeqEnb, lEchoSeqData
  iEchoSeqEnb = 1
  lEchoSeqData = []

def fCheckEchoSeq(expdata):
  global iEchoSeqEnb
  iEchoSeqEnb = 0
  if lEchoSeqData != expdata:
    Error('Bad command echo sequence')

def fBuildEchoSeq(block):
  global lEchoSeqData
  if iEchoSeqEnb:
    lEchoSeqData.append( (block.Opcode, block.Macro, block.Result) )

oEchoMon = Monitor.Monitor('Echo Seq', fBuildEchoSeq)
bEcho.Monitors.append(oEchoMon)

