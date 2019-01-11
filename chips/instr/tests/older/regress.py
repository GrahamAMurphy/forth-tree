from __main__ import *
from random import random
import Monitor
import Sequencer

# ------------------------------------------------------------------------
# Select instrument

# instr = 'MDS'; host = 'MESS'
instr = 'LOR'; host = 'NH'
# instr = 'PEP'; host = 'NH'
# instr = 'CRM'; host = 'MRO'

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

  fLoadFile = i_LORRI.lor_com_cmds.fLOR_LOAD_FILE

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
  CMD(instr + '_' + dTrueNames.get(name, name), args)

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

def fCheckEcho(oSeq, cmdname, result, args):
  if fPacketSeen(oSeq, bEcho, 10):
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
  lEchoSeqData.append( (block.Opcode, block.Macro, block.Result) )

oEchoMon = Monitor.Monitor('Echo Seq', fBuildEchoSeq)
bEcho.Monitors.append(oEchoMon)

# ------------------------------------------------------------------------
# Basic commanding

cmd_executed = 0
cmd_rejected = 0

def fSyncCmd():
  global cmd_executed, cmd_rejected
  cmd_executed = bStatus.CmdExec
  cmd_rejected = bStatus.CmdReject

def fClrCmdExec():
  global cmd_executed
  cmd_executed = 0

def fClrCmdReject():
  global cmd_rejected
  cmd_rejected = 0

def fGoodCmd():
  global cmd_executed
  cmd_executed = (cmd_executed + 1) % 256

def fBadCmd():
  global cmd_rejected
  cmd_rejected = (cmd_rejected + 1) % 256

def fCheckCmdCounts(oSeq):
  if fPacketSeen(oSeq, bStatus, 10):
    if not((bStatus.CmdExec==cmd_executed) and
           (bStatus.CmdReject==cmd_rejected)):
      Error('Bad command counts')
      fSyncCmd()
  else:
    Error('No status')

def fClrCounts(oSeq):
  # Precondition: status interval set to a high rate.
  C('---STAT_CLR(255) clears all, establishes preconditions---')
  fSendCmd('STAT_CLR', 255)
  fCheckEcho(oSeq, 'STAT_CLR', 0, [255])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.CmdExec==1) and
				(bStatus.CmdReject==0) and
				(bStatus.MacCmdExec==0) and
				(bStatus.MacCmdReject==0))
  fSyncCmd()				# save command counters

def fBasicCommand(oSeq):
  # Precondition: status interval set to a high rate.
  fSyncCmd()
  C('---Basic command functionality---')
  C('---NULL command---')
  fSendCmd('CMD_NULL')
  fGoodCmd()
  fCheckEcho(oSeq, 'CMD_NULL', 0, [])
  fCheckCmdCounts(oSeq)
  C('---WRAPped NULL command---')
  if host=='NH':			# LORRI/PEPSSI-specific
    fSendCmd('CMD_WRAP', 0, dOpcodes['CMD_NULL'], 0, 0)
  elif host=='MRO':			# MRO-specific
    fSendCmd('CMD_WRAP', dOpcodes['CMD_NULL'])
  else:					# MESSENGER-specific
    # TBD: MDIS-specific
    fSendCmd('CMD_WRAP', 6, dOpcodes['CMD_NULL'], 0, 0)
  fGoodCmd()
  fCheckEcho(oSeq, 'CMD_NULL', 0, [])
  fCheckCmdCounts(oSeq)
  C('---undefined command---')
  fSendCmd('ROM_BOOT')
  fBadCmd()
  if host=='MRO':			# MRO-specific; echoes 2-byte pad
    fCheckEcho(oSeq, 'ROM_BOOT', 2, [0,0])
  else:
    fCheckEcho(oSeq, 'ROM_BOOT', 2, [])
  fCheckCmdCounts(oSeq)
  C('---Check STAT_INT command with different intervals---')
  for interval in [1, 2, 3, 10]:
    C('\tinterval = %d' % interval)
    fSendCmd('STAT_INT', interval)
    fGoodCmd()
    fCheckEcho(oSeq, 'STAT_INT', 0, [0,interval])
    fCheckCmdCounts(oSeq)
    fCheckSeq(oSeq, bStatus, 4, interval)
  C('---STAT_INT(0) turns off status---')
  fSendCmd('STAT_INT', 0)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_INT', 0, [0,0])
  oSeq.Sleep(2)				# wait for status to be off
  fCheckNoPacket(oSeq, bStatus)
  C('---Use STAT_INT(1) for rest of test---')
  fSendCmd('STAT_INT', 1)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_INT', 0, [0,1])
  fCheckCmdCounts(oSeq)
  C('---STAT_CLR(0) clears commands executed---')
  fSendCmd('STAT_CLR', 0)
  fClrCmdExec(); fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [0])
  fCheckCmdCounts(oSeq)
  C('---STAT_CLR(1) clears commands rejected---')
  fSendCmd('STAT_CLR', 1)
  fClrCmdReject(); fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [1])
  fCheckCmdCounts(oSeq)
  C('---STAT_CLR(255) clears all---')
  fSendCmd('STAT_CLR', 255)
  fClrCmdExec(); fClrCmdReject(); fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [255])
  fCheckCmdCounts(oSeq)

# ------------------------------------------------------------------------
# Memory management

def fMemory(oSeq):
  # Precondition: appropriate .sim file loaded into GSEOS
  # Precondition: status interval set to a high rate.
  # TBD: doesn't test MEM_RUN
  C('---Memory management---')
  C('---MEM_WR_EN to disable memory writes---')
  fSendCmd('MEM_WR_EN', 0)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_WR_EN', 0, [0x00])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MemWrite==0))
  fCheckCmdCounts(oSeq)
  C('---STAT_MEM sent to monitor test memory location---')
  fSendCmd('STAT_MEM', 0x70000)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_MEM', 0, [0x00,0x07,0x00,0x00])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.WatchMemId==0x7) and
  				(bStatus.WatchAddr==0x0000))
  fCheckCmdCounts(oSeq)
  C('---MEM_LOAD should be rejected; write disabled---')
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_LOAD', 0x70000, 0, 1, 2, 3, 4) # TBD: work around GSEOS bug
  else:
    fSendCmd('MEM_LOAD', 0x70000, 1, 2, 3, 4) # TBD: work around GSEOS bug
  fBadCmd()
  fCheckEcho(oSeq, 'MEM_LOAD', 0x0a,
		[0x00, 0x07, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x01, 0x02])
  fCheckCmdCounts(oSeq)
  C('---MEM_COPY should be rejected; write disabled---')
  fSendCmd('MEM_COPY', 0, 0x70000, 16)
  fBadCmd()
  fCheckEcho(oSeq, 'MEM_COPY', 0x0a,
		[0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x10])
  fCheckCmdCounts(oSeq)
  C('---MEM_WR_EN to enable memory writes---')
  fSendCmd('MEM_WR_EN', 1)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_WR_EN', 0, [0x01])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MemWrite==1))
  fCheckCmdCounts(oSeq)
  C('---MEM_LOAD should be accepted now---')
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_LOAD', 0x70000, 0, 1, 2, 3, 4) # TBD: work around GSEOS bug
  else:
    fSendCmd('MEM_LOAD', 0x70000, 1, 2, 3, 4) # TBD: work around GSEOS bug
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_LOAD', 0x00,
		[0x00, 0x07, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x01, 0x02])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.WatchData[0]==0x01) and
  				(bStatus.WatchData[1]==0x02))
  fCheckCmdCounts(oSeq)
  C('---MEM_COPY should be accepted now---')
  fSendCmd('MEM_COPY', 0, 0x70000, 16)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_COPY', 0x00,
		[0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x10])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.WatchData[0]!=0x01) and
  				(bStatus.WatchData[1]!=0x02))
  fCheckCmdCounts(oSeq)
  C('---Load a file---')
  fLoadFile('/jrh/messfor.mem', 0x70000)
  oSeq.Sleep(15)			# TBD: wait for load to complete
  fSyncCmd()				# sync up command counters
  C('---MEM_CHECK used to verify successful load---')
  fSendCmd('MEM_CHECK', 0x70000, 6150)
  fGoodCmd()
# fCheckEcho(oSeq, 'MEM_CHECK', 0x00,	# there is a race between echo and check
#		[0x00, 0x07, 0x00, 0x00, 0x18, 0x06])
  fCheckMemCheck(oSeq, 0x70000, 6150, 0x8a02)
  fCheckCmdCounts(oSeq)
  C('---MEM_READ used to peek at loaded data---')
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_READ', 0x70000, 16, 0)
  else:
    fSendCmd('MEM_READ', 0x70000, 16)
  fGoodCmd()
# fCheckEcho(oSeq, 'MEM_READ', 0x00,	# there is a race between echo and read
#		[0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10])
  fCheckMemRead(oSeq, 0x70000, 0x70000, 16,
  			[0xbe, 0x04, 0xbe, 0x03, 0xb0, 0x0d, 0xa0, 0x00,
			 0xbe, 0x83, 0xbe, 0xa4, 0xa0, 0x20, 0x00, 0x00])
  fCheckCmdCounts(oSeq)
  C('---MEM_READ used to dump entire page---')
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_READ', 0x00000, 0xffff, 0)
  else:
    fSendCmd('MEM_READ', 0x00000, 0xffff)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_READ', 0x00,
		[0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff])
  fCheckMemRead(oSeq, 0x00000, 0x01000, 256, [])
  fCheckSeq(oSeq, bMemRead, 10, 1)
  fCheckCmdCounts(oSeq)
  C('---MEM_READ aborts last dump, starts new dump---')
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_READ', 0x10000, 0xffff, 0)
  else:
    fSendCmd('MEM_READ', 0x10000, 0xffff)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_READ', 0x00,
		[0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff])
  oSeq.Sleep(3)
  fCheckMemRead(oSeq, 0x10000, 0x11000, 256, [])
  fCheckSeq(oSeq, bMemRead, 10, 1)
  fCheckCmdCounts(oSeq)
  if host=='NH':			# LORRI/PEPSSI-specific
    C('---Test NH dump packet handshake---')
    fSetTime(100); oSeq.Sleep(3)	# rewind time so covered by sim file
    fCheckSeq(oSeq, bMemRead, 10, 2)	# should be every 2 seconds
  elif host=='MESS':			# MESSENGER-specific
    C('---Test MESSENGER dump packet rate control---')
    C('---MEM_READ_INT(2) produces packets every 2 seconds---')
    fSendCmd('MEM_READ_INT', 2)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x02])
    oSeq.Sleep(3)
    fCheckSeq(oSeq, bMemRead, 10, 2)	# should be every 2 seconds
    fCheckCmdCounts(oSeq)
    C('---MEM_READ_INT(0) turns off packet production---')
    fSendCmd('MEM_READ_INT', 0)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x00])
    oSeq.Sleep(3)
    fCheckNoPacket(oSeq, bMemRead)
    fCheckCmdCounts(oSeq)
    C('---MEM_READ_INT(1) produces packets every second---')
    fSendCmd('MEM_READ_INT', 1)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x01])
    oSeq.Sleep(3)
    fCheckSeq(oSeq, bMemRead, 10, 1)	# should be every second
    fCheckCmdCounts(oSeq)
  C('---MEM_READ_ABRT aborts memory dump---')
  fSendCmd('MEM_READ_ABRT')
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_READ_ABRT', 0x00, [])
  oSeq.Sleep(2)				# wait for dump to stop
  fCheckNoPacket(oSeq, bMemRead)
  fCheckCmdCounts(oSeq)
  C('---STAT_CLR(4) clears telemetry volume---')
  fSendCmd('STAT_CLR', 4)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [4])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.TlmVolume==0))
  fCheckCmdCounts(oSeq)
  C('---MEM_READ used to produce known telemetry volume---')
  # TBD/BUG: assumes no other telemetry source; in LORRI, image descriptors flow
  fSendCmd('STAT_INT', 0)		# turn off status for now
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_INT', 0, [0,0])
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_READ', 0x70000, 4096, 0)
  else:
    fSendCmd('MEM_READ', 0x70000, 4096)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_READ', 0x00,
		[0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00])
  oSeq.Sleep(16)			# wait for dump to complete
  fSendCmd('STAT_INT', 1)		# turn status back on
  fGoodCmd()
  fCheckPacket(oSeq, bStatus, lambda: ((bStatus.TlmVolume>=4) and
				bStatus.TlmVolume<=5))
  fCheckCmdCounts(oSeq)

# ------------------------------------------------------------------------
# Macro tests

iMacBlocks = 0

def fMacroControlFlow(oSeq):
  # Precondition: status interval set to a high rate.
  # Precondition: need clean slate of undefined macros for test to work
  C('---Macros: definition and control flow---')
  global iMacBlocks
  iMacBlocks = bStatus.MacroBlocks	# save number of macro blocks
  fClrCounts(oSeq)			# reset command counters
  C('---MAC_DEF(100) starts macro definitions---')
  fSendCmd('MAC_DEF', 100)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [100])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacLearn==1))
  C('---MAC_DEF(101, EXE) should be rejected---')
  common.config.fSetMacroMode("EXE")
  fSendCmd('MAC_DEF', 101)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 6, [101])
  fCheckCmdCounts(oSeq)
  C('---MAC_RUN(100) should fail because macro definition is not complete---')
  common.config.fSetMacroMode("EXE")
  fSendCmd('MAC_RUN', 100)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_RUN', 3, [100])
  fCheckCmdCounts(oSeq)
  # common.config.fSetMacroMode("APPEND")
  C('---MAC_ENDDEF() ends macro definition---')
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: ((bStatus.MacLearn==0) and
				bStatus.MacroBlocks==iMacBlocks-1))
  C('---MAC_RUN(100) runs macro---')
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 100)
  fGoodCmd()
  oSeq.Sleep(4)
  fCheckEchoSeq([
	(dOpcodes['MAC_RUN'], 0, 0),
	(dOpcodes['MAC_END'], 1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==1) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==100))
  C('---MAC_ENDDEF() should be rejected if no macro being defined---')
  fSendCmd('MAC_ENDDEF')
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 6, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_END() should be rejected if used outside of a macro---')
  fSendCmd('MAC_END')
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_END', 5, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_NEST(100) should be rejected if used outside of a macro---')
  fSendCmd('MAC_NEST', 100)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_NEST', 5, [100])
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(101), MAC_NEST(99), MAC_ENDDEF() can be defined')
  C('   even though macro 99 is not defined yet---')
  fSendCmd('MAC_DEF', 101)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [101])
  fSendCmd('MAC_NEST', 99)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_NEST', 1, [99])
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacroBlocks==iMacBlocks-2))
  C('---MAC_RUN(101) runs, but MAC_NEST(99) is rejected---')
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 101)
  fGoodCmd()
  oSeq.Sleep(4)
  fCheckEchoSeq([
	(dOpcodes['MAC_RUN'],  0, 0),
	(dOpcodes['MAC_NEST'], 1, 3),
	(dOpcodes['MAC_END'],  1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==2) and
				(bStatus.MacCmdReject==1) and
				(bStatus.MacId==101))
  C('---MAC_DEF(101), MAC_NEST(100), MAC_ENDDEF() redefines macro 101')
  C('   to call macro 100 instead of macro 99---')
  fSendCmd('MAC_DEF', 101)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [101])
  fSendCmd('MAC_NEST', 100)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_NEST', 1, [100])
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus,				# no change
			lambda: (bStatus.MacroBlocks==iMacBlocks-2))
  C('---MAC_RUN(101) runs successfully---')
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 101)
  fGoodCmd()
  oSeq.Sleep(4)
  fCheckEchoSeq([
	(dOpcodes['MAC_RUN'],  0, 0),
	(dOpcodes['MAC_NEST'], 1, 0),
	(dOpcodes['MAC_END'],  1, 0),
	(dOpcodes['MAC_END'],  1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==5) and
				(bStatus.MacCmdReject==1) and
				(bStatus.MacId==101))
  C('---MAC_RUN_SILENT(101) runs silently---')
  fStartEchoSeq()
  fSendCmd('MAC_RUN_SILENT', 101)
  fGoodCmd()
  oSeq.Sleep(4)
  fCheckEchoSeq([
	(dOpcodes['MAC_RUN_SILENT'],  0, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==5) and
				(bStatus.MacCmdReject==1) and
				(bStatus.MacId==101))
  C('---STAT_CLR(2) clears macro command execute counter---')
  fSendCmd('STAT_CLR', 2)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [2])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==0) and
				(bStatus.MacCmdReject==1))
  fCheckCmdCounts(oSeq)
  C('---STAT_CLR(3) clears macro command reject counter---')
  fSendCmd('STAT_CLR', 3)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [3])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==0) and
				(bStatus.MacCmdReject==0))
  fCheckCmdCounts(oSeq)

def fMacroTools(oSeq):
  # Precondition: status interval set to a high rate.
  C('---Macros: delay, pause, halt, etc.---')
  fClrCounts(oSeq)			# reset command counters
  C('---MAC_DELAY(10) should be rejected if used outside macro definition---')
  fSendCmd('MAC_DELAY', 10)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_DELAY', 5, [0,10])
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(102), MAC_DELAY(10), MAC_ENDDEF() defined---')
  fSendCmd('MAC_DEF', 102)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [102])
  fSendCmd('MAC_DELAY', 10)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DELAY', 1, [0,10])
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_RUN(102) runs successfully until DELAY---')
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 102)
  fGoodCmd()
  oSeq.Sleep(4)				# wait for first part of macro to run
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_RUN'],   0, 0),
	(dOpcodes['MAC_DELAY'], 1, 0)])
  C('---MAC_RUN(102) completes successfully---')
  fStartEchoSeq()
  oSeq.Sleep(10)			# wait for rest of macro to run
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_END'],   1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==2) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==102))
  C('---MAC_PAUSE() should be rejected if used outside macro definition---')
  fSendCmd('MAC_PAUSE', 0)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_PAUSE', 5, [0,0,0,0])
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(102), MAC_PAUSE(), MAC_ENDDEF() defined---')
  fSendCmd('MAC_DEF', 102)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [102])
  fSendCmd('MAC_PAUSE', 0x1234)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_PAUSE', 1, [0,0,0x12,0x34])
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_RUN(102) runs successfully until PAUSE---')
  fSetTime(100); oSeq.Sleep(3)		# rewind time
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 102)
  fGoodCmd()
  oSeq.Sleep(4)				# wait for first part of macro to run
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_RUN'],   0, 0),
	(dOpcodes['MAC_PAUSE'], 1, 0)])
  C('---MAC_RUN(102) completes successfully---')
  fStartEchoSeq()
  fSetTime(0x1234)			# move time forward
  oSeq.Sleep(4+2)			#   wait for rest of macro to run
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_END'],   1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==4) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==102))
  C('---MAC_HALT(103) should be rejected because macro is undefined---')
  fSendCmd('MAC_HALT', 103)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_HALT', 3, [103])
  fCheckCmdCounts(oSeq)
  C('---MAC_HALT(102) should be rejected because macro is not running---')
  fSendCmd('MAC_HALT', 102)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_HALT', 7, [102])
  fCheckCmdCounts(oSeq)
  C('---Reset time, then MAC_RUN(102).  MAC_HALT(102) should kill macro---')
  fSetTime(100); oSeq.Sleep(3)		# move time back
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 102)
  fGoodCmd()
  oSeq.Sleep(3)				# wait for first part of macro to run
  fSendCmd('MAC_HALT', 102)		#   kill macro
  fGoodCmd()
  oSeq.Sleep(4+2)				# wait for echos to arrive
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_RUN'],   0, 0),
	(dOpcodes['MAC_PAUSE'], 1, 0),
	(dOpcodes['MAC_HALT'],  0, 0)])
  fStartEchoSeq()
  fSetTime(0x1234); oSeq.Sleep(10)	# move time forward
  fCheckEchoSeq([])			#   and check echos (should be none)
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==5) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==102))

def fMacroLoops(oSeq):
  # Precondition: status interval set to a high rate.
  # Precondition: fMacroTools run, to define macro 102
  C('---Macros: loops, etc.---')
  fClrCounts(oSeq)			# reset command counters
  C('---MAC_LOOP_BEGIN() should be rejected if used outside macro defn.---')
  fSendCmd('MAC_LOOP_BEGIN', 10)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_LOOP_BEGIN', 5, [0,10])
  fCheckCmdCounts(oSeq)
  C('---MAC_LOOP_END() should be rejected if used outside macro defn.---')
  fSendCmd('MAC_LOOP_END')
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_LOOP_END', 5, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(103), MAC_LOOP_BEGIN(5), MAC_LOOP_END(), and MAC_ENDDEF()---')
  fSendCmd('MAC_DEF', 103)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [103])
  fSendCmd('MAC_LOOP_BEGIN', 5)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_LOOP_BEGIN', 1, [0,5])
  fSendCmd('MAC_LOOP_END')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_LOOP_END', 1, [])
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_RUN(103) runs successfully---')
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 103)
  fGoodCmd()
  oSeq.Sleep(4)
  fCheckEchoSeq([
	(dOpcodes['MAC_RUN'],        0, 0),
	(dOpcodes['MAC_LOOP_BEGIN'], 1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_END'],        1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==7) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==103))
  C('---MAC_DEF(103), MAC_LOOP_BEGIN(63), MAC_RUN(102),')
  C('   MAC_LOOP_END(), and MAC_ENDDEF() redefines macro 103---')
  fSendCmd('MAC_DEF', 103)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [103])
  fSendCmd('MAC_LOOP_BEGIN', 63)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_LOOP_BEGIN', 1, [0,63])
  fSendCmd('MAC_RUN', 102)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_RUN', 1, [102])
  fSendCmd('MAC_LOOP_END')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_LOOP_END', 1, [])
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  C('---Reset time.  MAC_RUN(103) forks 63 macros; all PAUSE---')
  fSetTime(100); oSeq.Sleep(3)		# rewind time
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 103)
  fGoodCmd()
  oSeq.Sleep(10)
  fCheckEchoSeq([
	(dOpcodes['MAC_RUN'],        0, 0),
	(dOpcodes['MAC_LOOP_BEGIN'], 1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_END'],        1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0) ])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==198) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==102))
  C('---Advance time; PAUSEd macros complete---')
  fStartEchoSeq()
  fSetTime(0x1234); oSeq.Sleep(10)	# move time forward
  fCheckEchoSeq([
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0) ])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==5) and # counts wrap
				(bStatus.MacCmdReject==0))
  C('---Reset time.  MAC_RUN(103) forks 63 macros; all PAUSE---')
  fSetTime(100); oSeq.Sleep(3)		# rewind time
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 103)
  fGoodCmd()
  oSeq.Sleep(10)
  fCheckEchoSeq([
	(dOpcodes['MAC_RUN'],        0, 0),
	(dOpcodes['MAC_LOOP_BEGIN'], 1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_RUN'],        1, 0),
	(dOpcodes['MAC_LOOP_END'],   1, 0),
	(dOpcodes['MAC_END'],        1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0),
	(dOpcodes['MAC_PAUSE'],      1, 0) ])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==196) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==102))
  C('---MAC_RUN(102) runs until PAUSE; 64 macros running---')
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 102)
  fGoodCmd()
  oSeq.Sleep(4)				# wait for first part of macro to run
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_RUN'],   0, 0),
	(dOpcodes['MAC_PAUSE'], 1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==197) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==102))
  C('---MAC_RUN(102) should fail; out of macro contexts---')
  fSendCmd('MAC_RUN', 102)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_RUN', 4, [102])
  fCheckCmdCounts(oSeq)
  C('---Advance time; all 64 PAUSEd macros complete---')
  fStartEchoSeq()
  fSetTime(0x1234); oSeq.Sleep(10)	# move time forward
  fCheckEchoSeq([
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0),
	(dOpcodes['MAC_END'], 1, 0) ])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==5) and # counts wrap
				(bStatus.MacCmdReject==0))

def fMacroDump(oSeq):
  # Precondition: status interval set to a high rate.
  # Precondition: need clean slate of undefined macros for test to work
  C('---Macros: dump, check, etc.---')
  fClrCounts(oSeq)			# reset command counters
  C('---MAC_READ(200,200) of undefined macro produces no packets---')
  fSendCmd('MAC_READ', 200, 200)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_READ', 0, [200,200])
  fCheckNoPacket(oSeq, bMacRead)
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(200,200) of undefined macro produces default checksum---')
  fSendCmd('MAC_CHECK', 200, 200)
  fGoodCmd()
  #fCheckEcho(oSeq, 'MAC_CHECK', 0, [200,200])	# race between echo and check
  fCheckMacCheck(oSeq, 200, 200, [0xffff])
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(200), CMD_NULL(), MAC_ENDDEF() defined---')
  fSendCmd('MAC_DEF', 200)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [200])
  fSendCmd('CMD_NULL')
  fGoodCmd()
  fCheckEcho(oSeq, 'CMD_NULL', 1, [])
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  m200 = [2, 0, dOpcodes['CMD_NULL'], 2, 0, dOpcodes['MAC_END']]
  C('---MAC_READ(200,200) produces macro read packet---')
  fSendCmd('MAC_READ', 200, 200)
  fGoodCmd()
  #fCheckEcho(oSeq, 'MAC_READ', 0, [200,200])	# race between echo and read
  fCheckMacRead(oSeq, 200, 0, m200)
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(200,200) produces checksum---')
  fSendCmd('MAC_CHECK', 200, 200)
  fGoodCmd()
  #fCheckEcho(oSeq, 'MAC_CHECK', 0, [200,200])	# race between echo and check
  fCheckMacCheck(oSeq, 200, 200, [fMacChecksum(m200)])
  fCheckCmdCounts(oSeq)
  C('---MAC_READ(200,209) produces macro read packet, skips undefined---')
  fSendCmd('MAC_READ', 200, 209)
  fGoodCmd()
  fCheckMacRead(oSeq, 200, 0, m200)
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(200,209) produces checksums, default for undefined---')
  fSendCmd('MAC_CHECK', 200, 209)
  fGoodCmd()
  fCheckMacCheck(oSeq, 200, 209, [fMacChecksum(m200)]+[0xffff]*9)
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(201), 10*CMD_NULL(), MAC_ENDDEF() defines multiblock macro---')
  fSendCmd('MAC_DEF', 201)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [201])
  for i in range(10):
    fSendCmd('CMD_NULL')
    fGoodCmd()
    fCheckEcho(oSeq, 'CMD_NULL', 1, [])
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  m201 = [2, 0, dOpcodes['CMD_NULL'], 2, 0, dOpcodes['CMD_NULL'],
  		2, 0, dOpcodes['CMD_NULL'], 2, 0, dOpcodes['CMD_NULL'],
  		2, 0, dOpcodes['CMD_NULL'], 2, 0, dOpcodes['CMD_NULL'],
  		2, 0, dOpcodes['CMD_NULL'], 2, 0, dOpcodes['CMD_NULL'],
  		2, 0, dOpcodes['CMD_NULL'], 2, 0, dOpcodes['CMD_NULL'],
		2, 0, dOpcodes['MAC_END']]
  C('---MAC_READ(200,209) produces macro read packets for 200 and 201---')
  fSendCmd('MAC_READ', 200, 209)
  fGoodCmd()
  fCheckMacRead(oSeq, 200, 0, m200)
  fCheckMacRead(oSeq, 201, 0, m201[0:32])
  fCheckMacRead(oSeq, 201, 1, m201[32:64])
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(200,209) produces checksums, default for undefined---')
  fSendCmd('MAC_CHECK', 200, 209)
  fGoodCmd()
  fCheckMacCheck(oSeq, 200, 209,
		[fMacChecksum(m200),fMacChecksum(m201)]+[0xffff]*8)
  fCheckCmdCounts(oSeq)
  C('---MAC_SAVE() command---')
  fSendCmd('MAC_SAVE')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_SAVE', 0, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(*), MAC_ENDDEF() defines all macros---')
  for i in range(256):
    fSendCmd('MAC_DEF', i)
    fGoodCmd()
    fCheckEcho(oSeq, 'MAC_DEF', 0, [i])
    fSendCmd('MAC_ENDDEF')
    fGoodCmd()
    fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  mall = [2, 0, dOpcodes['MAC_END']]
  C('---MAC_READ(0,255) used to dump all macros---')
  fSendCmd('MAC_READ', 0, 255)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_READ', 0x00, [0,255])
  fCheckSeq(oSeq, bMacRead, 10, 1)
  fCheckCmdCounts(oSeq)
  if host=='NH':			# LORRI/PEPSSI-specific
    C('---Test NH dump packet handshake---')
    fSetTime(100); oSeq.Sleep(3)	# rewind time so covered by sim file
    fCheckSeq(oSeq, bMacRead, 10, 2)	# should be every 2 seconds
  elif host=='MESS':			# MESSENGER-specific
    C('---Test MESSENGER dump packet rate control---')
    C('---MEM_READ_INT(2) produces packets every 2 seconds---')
    fSendCmd('MEM_READ_INT', 2)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x02])
    oSeq.Sleep(3)
    fCheckSeq(oSeq, bMacRead, 10, 2)	# should be every 2 seconds
    fCheckCmdCounts(oSeq)
    C('---MEM_READ_INT(0) turns off packet production---')
    fSendCmd('MEM_READ_INT', 0)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x00])
    oSeq.Sleep(3)
    fCheckNoPacket(oSeq, bMacRead)
    fCheckCmdCounts(oSeq)
    C('---MEM_READ_INT(1) produces packets every second---')
    fSendCmd('MEM_READ_INT', 1)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x01])
    oSeq.Sleep(3)
    fCheckSeq(oSeq, bMacRead, 10, 1)	# should be every second
    fCheckCmdCounts(oSeq)
  C('---MAC_READ(1,0) aborts macro dump---')	# undocumented feature
  fSendCmd('MAC_READ', 1, 0)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_READ', 0x00, [1,0])
  oSeq.Sleep(2)				# wait for dump to stop
  fCheckNoPacket(oSeq, bMacRead)
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(0,255) used to check all macros---')
  fSendCmd('MAC_CHECK', 0, 255)
  fGoodCmd()
  fCheckMacCheck(oSeq, 0, 255, [fMacChecksum(mall)]*256)
  fCheckCmdCounts(oSeq)
  C('---MAC_RESTORE() command---')
  fSendCmd('MAC_RESTORE')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_RESTORE', 0, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(200,209) produces checksums for restored macros---')
  fSendCmd('MAC_CHECK', 200, 209)
  fGoodCmd()
  fCheckMacCheck(oSeq, 200, 209,
		[fMacChecksum(m200),fMacChecksum(m201)]+[0xffff]*8)
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(*), STAT_INT(random#), MAC_ENDDEF() defines all macros---')
  sums = [0]*256
  for i in range(256):
    interval = int(65535*random())
    intms = (interval>>8)&0xff; intls = interval&0xff
    fSendCmd('MAC_DEF', i)
    fGoodCmd()
    fCheckEcho(oSeq, 'MAC_DEF', 0, [i])
    fSendCmd('STAT_INT', interval)
    fGoodCmd()
    fCheckEcho(oSeq, 'STAT_INT', 1, [intms, intls])
    fSendCmd('MAC_ENDDEF')
    fGoodCmd()
    fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
    sums[i] = fMacChecksum([4, 0, dOpcodes['STAT_INT'], intms, intls,
				2, 0, dOpcodes['MAC_END']])
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(0,255) used to check all macros---')
  fSendCmd('MAC_CHECK', 0, 255)
  fGoodCmd()
  fCheckMacCheck(oSeq, 0, 255, sums)
  fCheckCmdCounts(oSeq)

def fMacros(oSeq):
  # Precondition: status interval set to a high rate.
  C('---Macros---')
  fMacroControlFlow(oSeq)
  fMacroTools(oSeq)
  fMacroLoops(oSeq)
  fMacroDump(oSeq)

# ------------------------------------------------------------------------
# Safing

def fSafing(oSeq):
  # Precondition: macro tests passed
  # Precondition: status interval set to a high rate.
  C('---Safing---')
  fClrCounts(oSeq)			# clear command counters
  C('---MAC_DEF(1), MAC_NEST(2), MAC_ENDDEF() replaces shutdown macro')
  fSendCmd('MAC_DEF', 1)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [1])
  fSendCmd('MAC_NEST', 2)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_NEST', 1, [2])
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(2), MAC_ENDDEF() replaces safing macro')
  fSendCmd('MAC_DEF', 2)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [2])
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  C('---SAFE() runs safing macro---')
  fStartEchoSeq()
  fSendCmd('SAFE')
  fGoodCmd()
  oSeq.Sleep(4)
  fCheckEchoSeq([
	(dOpcodes['SAFE'],    0, 0),
	(dOpcodes['MAC_END'], 1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==1) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==2))
  C('---SHUTDOWN() runs shutdown macro---')
  fStartEchoSeq()
  fSendCmd('SHUTDOWN')
  fGoodCmd()
  oSeq.Sleep(4)
  fCheckEchoSeq([
	(dOpcodes['SHUTDOWN'], 0, 0),
	(dOpcodes['MAC_NEST'], 1, 0),
	(dOpcodes['MAC_END'],  1, 0),
	(dOpcodes['MAC_END'],  1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==4) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==1))
# TBD: won't work if no boot ROM!
# C('---SOFT_RESET() causes watchdog reset back to boot---')
# fSendCmd('SOFT_RESET')		# no command echo
# TBD: add function parameter ...
# fCheckPacket(oSeq, bBootStatus)
# TBD: missing PWR_OFF check

# ------------------------------------------------------------------------
# Top level test

def fRegression(oSeq):
# TBD: Preconditions:
  C('---Starting regression test---')
  fBasicCommand(oSeq)
  #fMemory(oSeq)
  #fMacros(oSeq)
  #if host!='MRO':			# not supported on CRISM
    #fSafing(oSeq)
  C('---Regression test done---')
  ErrorReport()

Sequencer.Sequencer('Common Code Regression', fRegression)
