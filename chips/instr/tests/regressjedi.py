# (c) 2003 Johns Hopkins University / Applied Physics Laboratory
# by John R. Hayes
from __main__ import *
import GseosSequencer as Sequencer
from regresstools import *

# ------------------------------------------------------------------------
# Basic commanding

cmd_executed = 0
cmd_rejected = 0
alarms_seen = 0

def fSyncCmd():
  global cmd_executed, cmd_rejected, alarms_seen
  cmd_executed = bStatus.CmdExec
  cmd_rejected = bStatus.CmdReject
  alarms_seen =  bStatus.AlarmCount

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
    fSendCmd('CMD_WRAP', dDests[instr], dOpcodes['CMD_NULL'], 0, 0)
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
  # TBD: fails on MDIS: analog alarms occur
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.AlarmCount==alarms_seen))

# ------------------------------------------------------------------------
# Top level test

def fRegression(oSeq):
# TBD: Preconditions:
  C('---Starting common regression test---')
  ResetError()
  fBasicCommand(oSeq)
  C('---Regression test done---')
  ErrorReport()

Sequencer.Sequencer('Common Code Regression', fRegression)
