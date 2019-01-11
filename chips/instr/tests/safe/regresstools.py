# (c) 2003 Johns Hopkins University / Applied Physics Laboratory
# by John R. Hayes
from __main__ import *

# ------------------------------------------------------------------------
# Select instrument

# instr = 'MDS'; cmdprefix = instr; host = 'MESS'
# instr = 'MAG'; cmdprefix = instr; host = 'MESS'
# instr = 'NUS'; cmdprefix = instr; host = 'MESS'
# instr = 'LOR'; cmdprefix = instr; host = 'NH'
# instr = 'PEP'; cmdprefix = instr; host = 'NH'
# instr = 'VSL'; cmdprefix = instr; host = 'NH'
# instr = 'ATAS'; cmdprefix = instr; host = 'NH'
# instr = 'CRM'; cmdprefix = instr; host = 'MRO'
from HostConstants import host, instr, cmdprefix

# ------------------------------------------------------------------------
# MESSENGER-specific things here

if host == 'MESS':
  # use GSEOS 5.2; emulate 6.0
  from Monitor import Monitor as TMonitor
  from Sequencer import Sequencer as TSequencer, \
    TimeoutError as TSeqTimeoutError
  from core.core import CMD
  from Common.MET import SetMET
  from Common.config import fSetMacroMode

  dDests = {
	'MDS' : 0x06,
	'EPP' : 0x08,
	'GRS' : 0x09,
	'NUS' : 0x0a,
	'MAG' : 0x0b,
	'XRS' : 0x0c,
	'MASCS' : 0x0d
}

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

  # Arguments: map to values
  enable=1					# Enable/Disable
  disable=0
  cmdexec=0					# Counters
  cmdrej=1
  maccmdexec=2
  maccmdrej=3
  telvol=4
  all=255

  # Programs to use in file load tests, etc.
  tstpg = 0x6
  progname = '/jrh/messfor.mem'
  progwords = 6150; progsum = 0x8a02
  progstart = 0x042a
  eeprogname = '/jrh/messfore.mem'
  eeprogwords = 6155; eeprogsum = 0x8269
  eeprogpg = 0xa

  def fSetTime(t):
    SetMET(t)

  def fPktSeqNums(block):
    return (block.SeqCount, block.MET)

  def fPktSeqNext(seq, time, interval):
    return (seq+1, time+interval)

  def fPktDataLen(blk):
    return blk.Length + 1 - 4

  def fLoadFile(path, addr):
    core.loadFile.fLOAD_FILE(path, instr, addr)

  def fEchoLength(args):			# no command echo padding
    return len(args)

# ------------------------------------------------------------------------
# LORRI/PEPSSI-specific things here

if host == 'NH':
  # use GSEOS 5.2; emulate 6.0
  from Monitor import Monitor as TMonitor
  from Sequencer import Sequencer as TSequencer, \
    TimeoutError as TSeqTimeoutError
  from common.core import CMD
  from sceAPI import SetMET
  from common.config import fSetMacroMode

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

  # Arguments: map to values
  enable=1					# Enable/Disable
  disable=0
  cmdexec=0					# Counters
  cmdrej=1
  maccmdexec=2
  maccmdrej=3
  telvol=4
  all=255

  # Programs to use in file load tests, etc.
  tstpg = 0x6
  progname = '/jrh/messfor.mem'
  progwords = 6150; progsum = 0x8a02
  progstart = 0x042a
  eeprogname = '/jrh/messfore.mem'
  eeprogwords = 6155; eeprogsum = 0x8269
  eeprogpg = 0xa

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

  def fEchoLength(args):			# no command echo padding
    return len(args)

# ------------------------------------------------------------------------
# CRISM-specific things here

if host == 'MRO':
  # use GSEOS 5.2; emulate 6.0
  from Monitor import Monitor as TMonitor
  from Sequencer import Sequencer as TSequencer, \
    TimeoutError as TSeqTimeoutError
  from core.core import CMD
  from core.CRMTiming import fSetTime
  from common.config import fSetMacroMode

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

  # Arguments: map to values
  enable=1					# Enable/Disable
  disable=0
  cmdexec=0					# Counters
  cmdrej=1
  maccmdexec=2
  maccmdrej=3
  telvol=4
  all=255

  # Programs to use in file load tests, etc.
  tstpg = 0x6
  progname = '/jrh/messfor.mem'
  progwords = 6150; progsum = 0x8a02
  progstart = 0x042a
  eeprogname = '/jrh/messfore.mem'
  eeprogwords = 6155; eeprogsum = 0x8269
  eeprogpg = 0xa

  def fPktSeqNums(block):
    return (0, block.Time_Tag)

  def fPktSeqNext(seq, time, interval):
    return (0, time+interval)

  def fPktDataLen(blk):
    return blk.Length + 1

  fLoadFile = core.loadFile.fLOAD_FILE

  def fEchoLength(args):			# no command echo padding
    return len(args)

# ------------------------------------------------------------------------
# Puck-specific things here

if (host == 'Juno') or (host == 'RBSP') or (host == 'MMS'):
  # use GSEOS 6.0
  from GseosMonitor import TMonitor
  from GseosSequencer import TSequencer, TSeqTimeoutError
  from HostCommands import fSetMacroMode, fSetTime
  from PuckCommands import CMD, fLoadFile

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
	'ROM_BOOT' : 0x1e,
	'ROM_GO' : 0x1f,
	'SAF_OFF' : 0x22,
	'SAF_RESET' : 0x23,
	'SAF_TIMEOUT' : 0x24,
	'STAT_CLR' : 0x1c,
	'STAT_INT' : 0x1d,
	'STAT_MEM' : 0x20,
  }

  dTrueNames = {
	'MAC_LOOP_BEGIN' : 'MAC_LOOP_BEG',
	'MAC_RUN_SILENT' : 'MAC_RUN_SIL',
	'MEM_READ_ABRT'  : 'MEM_READ_ABT'
  }

  # Arguments: map to strings
  enable='Enable'				# Enable/Disable
  disable='Disable'
  cmdexec='CmdExec'				# Counters
  cmdrej='CmdRej'
  maccmdexec='MacCmdExec'
  maccmdrej='MacCmdRej'
  telvol='TelVol'
  all='All'

  # Programs to use in file load tests, etc.
  tstpg = 0xf
  progname = '/jrh/puckfor.mem'
  progwords = 5217; progsum = 0x41da
  progstart = 0x0210
  eeprogname = '/jrh/puckfore.mem'
  eeprogwords = 5222; eeprogsum = 0xee7b
  eeprogpg = 0x1b

  def fPktSeqNums(block):
    return (0, block.TimeTag)

  def fPktSeqNext(seq, time, interval):
    return (0, time+interval)

  def fPktDataLen(blk):
    return blk.Length

  # host-specific things here ...
  if (host == 'RBSP') or (host == 'MMS'):
    def fEchoLength(args):			# some pad echos to even multiple
      l = len(args)
      if l%2 == 1:
        return l+1
      else:
        return l
  else:
    def fEchoLength(args):			# others do not pad
      return len(args)

# ------------------------------------------------------------------------
# Useful helpers

# Blocks -----------------------------------------------------------------

bAppStatus  = eval(instr + '_Status')
bBootStatus = eval(instr + '_Boot_Status')
bAlarm      = eval(instr + '_Alarm')
bEcho       = eval(instr + '_CmdEcho')
bMemCheck   = eval(instr + '_MemChecksum')
bMemRead    = eval(instr + '_MemDump')
bMacCheck   = eval(instr + '_MacroChecksum')
bMacRead    = eval(instr + '_MacroDump')

# Error/status logging ---------------------------------------------------

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

def ErrorsSeen():
  return ierrors

# Commanding --------------------------------------------------------------

def fSendCmd(name, *args):
  CMD(cmdprefix + '_' + dTrueNames.get(name, name), *args)

# Packet checking ---------------------------------------------------------

def fPacketSeen(oSeq, block, timeout):
  seen = 1
  try:
    oSeq.Wait(block, lambda:1, (), timeout)
  except TSeqTimeoutError:
    seen = 0
  return seen

def fSyncIdle(oSeq, timeout, block=bEcho):
  # TBD: value differs for each system
  if fPacketSeen(oSeq, block, 8):	# if block seen in round-trip time
    while fPacketSeen(oSeq, block, timeout):
      pass				# wait until no packets seen

def fSyncTime(oSeq, newtime, block):
  syncing = 1
  fSetTime(newtime)
  try:
    while syncing:
      oSeq.Wait(block, lambda:1, (), 10)
      seq,time = fPktSeqNums(block)
      if ((time >= newtime) and
         (time-newtime < 5)):
        syncing = 0
  except TSeqTimeoutError:
    Error('Cannot sync time')

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
  except TSeqTimeoutError:
    Error('Packet sequence timeout')

def fCheckAlarm(oSeq, alarmid, type, value):
  if fPacketSeen(oSeq, bAlarm, 10):
    if not((bAlarm.AlarmId==alarmid) and (bAlarm.AlarmType==type) and
           (bAlarm.Value==value)):
      Error('Bad alarm')
  else:
    Error('No alarm')

def fCheckEcho(oSeq, cmdname, result, args, timeout=10):
  if fPacketSeen(oSeq, bEcho, timeout):
    if not((bEcho.Opcode==dOpcodes[cmdname]) and (bEcho.Result==result) and
            (fPktDataLen(bEcho)-2==fEchoLength(args))):
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

bEcho.Monitors.append(
  TMonitor('Echo Seq', fBuildEchoSeq))

# ------------------------------------------------------------------------
# Construct sequencer to run test

def fDefTest(str, fun):
  TSequencer(str, fun, bStart=0)
