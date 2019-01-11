# (c) 2003 Johns Hopkins University / Applied Physics Laboratory
# by John R. Hayes
from __main__ import *
import Sequencer
from regresstools import *

# ------------------------------------------------------------------------
# Basic commanding

cmd_executed = 0
cmd_rejected = 0

def fSyncCmd():
  global cmd_executed, cmd_rejected
  cmd_executed = bBootStatus.CmdExec
  cmd_rejected = bBootStatus.CmdReject

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
  if fPacketSeen(oSeq, bBootStatus, 10):
    if not((bBootStatus.CmdExec==cmd_executed) and
           (bBootStatus.CmdReject==cmd_rejected)):
      Error('Bad command counts')
      fSyncCmd()
  else:
    Error('No status')

def fClrCounts(oSeq):
  # Precondition: status interval set to a high rate.
  C('---STAT_CLR(255) clears all, establishes preconditions---')
  fSendCmd('STAT_CLR', 255)
  fCheckEcho(oSeq, 'STAT_CLR', 0, [255])
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.CmdExec==1) and
				(bBootStatus.CmdReject==0))
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
  fSendCmd('MAC_RESTORE')
  fBadCmd()
  if host=='MRO':			# MRO-specific; echoes 2-byte pad
    fCheckEcho(oSeq, 'MAC_RESTORE', 2, [0,0])
  else:
    fCheckEcho(oSeq, 'MAC_RESTORE', 2, [])
  fCheckCmdCounts(oSeq)
  C('---Check STAT_INT command with different intervals---')
  for interval in [1, 2, 3, 10]:
    C('\tinterval = %d' % interval)
    fSendCmd('STAT_INT', interval)
    fGoodCmd()
    fCheckEcho(oSeq, 'STAT_INT', 0, [0,interval])
    fCheckCmdCounts(oSeq)
    fCheckSeq(oSeq, bBootStatus, 4, interval)
  C('---STAT_INT(0) turns off status---')
  fSendCmd('STAT_INT', 0)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_INT', 0, [0,0])
  oSeq.Sleep(2)				# wait for status to be off
  fCheckNoPacket(oSeq, bBootStatus)
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
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.AlarmCount==0))

# ------------------------------------------------------------------------
# Memory management

def fMemory(oSeq):
  # Precondition: status interval set to a high rate.
  C('---Memory management---')
  C('---MEM_WR_EN to disable memory writes---')
  fSendCmd('MEM_WR_EN', 0)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_WR_EN', 0, [0x00])
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.MemWrite==0))
  fCheckCmdCounts(oSeq)
  C('---STAT_MEM sent to monitor test memory location---')
  fSendCmd('STAT_MEM', tstpg<<16)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_MEM', 0, [0x00,tstpg,0x00,0x00])
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.WatchMemId==tstpg) and
  				(bBootStatus.WatchAddr==0x0000))
  fCheckCmdCounts(oSeq)
  C('---MEM_LOAD should be rejected; write disabled---')
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_LOAD', tstpg<<16, 0, 1, 2, 3, 4) # TBD: work around GSEOS bug
  else:
    fSendCmd('MEM_LOAD', tstpg<<16, 1, 2, 3, 4) # TBD: work around GSEOS bug
  fBadCmd()
  fCheckEcho(oSeq, 'MEM_LOAD', 0x0a,
		[0x00, tstpg, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x01, 0x02])
  fCheckCmdCounts(oSeq)
  C('---MEM_COPY should be rejected; write disabled---')
  fSendCmd('MEM_COPY', 0x80000, tstpg<<16, 16)
  fBadCmd()
  fCheckEcho(oSeq, 'MEM_COPY', 0x0a,
		[0x00, 0x08, 0x00, 0x00, 0x00, tstpg, 0x00, 0x00, 0x00, 0x10])
  fCheckCmdCounts(oSeq)
  C('---MEM_WR_EN to enable memory writes---')
  fSendCmd('MEM_WR_EN', 1)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_WR_EN', 0, [0x01])
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.MemWrite==1))
  fCheckCmdCounts(oSeq)
  C('---MEM_LOAD should be accepted now---')
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_LOAD', tstpg<<16, 0, 1, 2, 3, 4) # TBD: work around GSEOS bug
  else:
    fSendCmd('MEM_LOAD', tstpg<<16, 1, 2, 3, 4) # TBD: work around GSEOS bug
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_LOAD', 0x00,
		[0x00, tstpg, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x01, 0x02])
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.WatchData[0]==0x01) and
  				(bBootStatus.WatchData[1]==0x02))
  fCheckCmdCounts(oSeq)
  C('---MEM_COPY should be accepted now---')
  fSendCmd('MEM_COPY', 0x80000, tstpg<<16, 16)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_COPY', 0x00,
		[0x00, 0x08, 0x00, 0x00, 0x00, tstpg, 0x00, 0x00, 0x00, 0x10])
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.WatchData[0]!=0x01) and
  				(bBootStatus.WatchData[1]!=0x02))
  fCheckCmdCounts(oSeq)
  C('---Load a file---')
  fLoadFile('/jrh/messfor.mem', tstpg<<16)
  oSeq.Sleep(15)			# TBD: wait for load to complete
  fSyncCmd()				# sync up command counters
  C('---MEM_CHECK used to verify successful load---')
  fSendCmd('MEM_CHECK', tstpg<<16, 6150)
  fGoodCmd()
# fCheckEcho(oSeq, 'MEM_CHECK', 0x00,	# there is a race between echo and check
#		[0x00, tstpg, 0x00, 0x00, 0x18, 0x06])
  fCheckMemCheck(oSeq, tstpg<<16, 6150, 0x8a02)
  fCheckCmdCounts(oSeq)
  C('---MEM_READ used to peek at loaded data---')
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_READ', tstpg<<16, 16, 0)
  else:
    fSendCmd('MEM_READ', tstpg<<16, 16)
  fGoodCmd()
# fCheckEcho(oSeq, 'MEM_READ', 0x00,	# there is a race between echo and read
#		[0x00, tstpg, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10])
  fCheckMemRead(oSeq, tstpg<<16, tstpg<<16, 16,
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
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.TlmVolume==0))
  fCheckCmdCounts(oSeq)
  C('---MEM_READ used to produce known telemetry volume---')
  fSendCmd('STAT_INT', 0)		# turn off status for now
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_INT', 0, [0,0])
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_READ', tstpg<<16, 4096, 0)
  else:
    fSendCmd('MEM_READ', tstpg<<16, 4096)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_READ', 0x00,
		[0x00, tstpg, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00])
  oSeq.Sleep(16)			# wait for dump to complete
  fSendCmd('STAT_INT', 1)		# turn status back on
  fGoodCmd()
  fCheckPacket(oSeq, bBootStatus, lambda: ((bBootStatus.TlmVolume>=4) and
				bBootStatus.TlmVolume<=5))
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.AlarmCount==0))

# ------------------------------------------------------------------------
# Program load and boot

def fBoot(oSeq):
  # Precondition: status interval set to a high rate.
  C('---Program load and boot---')
  fClrCounts(oSeq)			# clear command counters
  C('---MEM_WR_EN to enable memory writes---')
  fSendCmd('MEM_WR_EN', 1)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_WR_EN', 0, [0x01])
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.MemWrite==1))
  fCheckCmdCounts(oSeq)
  C('---Load a file: vslfor.mem---')
  fLoadFile('/jrh/vslfor.mem', 0)
  oSeq.Sleep(15)			# TBD: wait for load to complete
  fSyncCmd()				# sync up command counters
  C('---MEM_CHECK used to verify successful load---')
  fSendCmd('MEM_CHECK', 0, 5253)
  fGoodCmd()
# fCheckEcho(oSeq, 'MEM_CHECK', 0x00,	# there is a race between echo and check
#		[0x00, tstpg, 0x00, 0x00, 0x18, 0x06])
  fCheckMemCheck(oSeq, 0, 5253, 0xe1cc)
  fCheckCmdCounts(oSeq)
  C('---MEM_RUN used to start program---')
  fSendCmd('MEM_RUN', 0x0210)		# no echo
  oSeq.MessageBoxModeless('Confirm Forth application.  Reset.')
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.Cause==0))
  fSyncCmd()				# sync up command counters
  C('---MEM_WR_EN to enable memory writes---')
  fSendCmd('MEM_WR_EN', 1)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_WR_EN', 0, [0x01])
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.MemWrite==1))
  fCheckCmdCounts(oSeq)
  C('---Load a file: vslfore.mem---')
  fLoadFile('/jrh/vslfore.mem', 0)
  oSeq.Sleep(15)			# TBD: wait for load to complete
  fSyncCmd()				# sync up command counters
  C('---MEM_CHECK used to verify successful load---')
  fSendCmd('MEM_CHECK', 0, 5258)
  fGoodCmd()
# fCheckEcho(oSeq, 'MEM_CHECK', 0x00,	# there is a race between echo and check
#		[0x00, tstpg, 0x00, 0x00, 0x18, 0x06])
  fCheckMemCheck(oSeq, 0, 5258, 0x377c)
  fCheckCmdCounts(oSeq)
  C('---MEM_COPY to save program to EEPROM---')
  fSendCmd('MEM_COPY', 0, 0xa0000, 0x4000)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_COPY', 0x00,
		[0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x40, 0x00])
  fCheckCmdCounts(oSeq)
  C('---ROM_GO used to start saved program---')
  fSendCmd('ROM_GO', 0xa0000)		# no echo
  oSeq.MessageBoxModeless('Confirm Forth application.  Type: 2 dpu-cfg!')
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.Cause==1))
  fSyncCmd()				# sync up command counters
  C('---ROM_BOOT used to start default program---')
  fSendCmd('ROM_BOOT')			# no echo
  oSeq.MessageBoxModeless('Confirm default application.')

# ------------------------------------------------------------------------
# Top level test

def fRegression(oSeq):
  # Precondition: /jrh/messfor.mem on host.
  # Precondition: /jrh/vslfor.mem and /jrh/vslfore.mem on host.
  # Precondition: if NH, 2sec.sim loaded into GSEOS.
  C('---Starting boot regression test---')
  ResetError()
  fBasicCommand(oSeq)
  fMemory(oSeq)
  fBoot(oSeq)
  C('---Regression test done---')
  ErrorReport()

Sequencer.Sequencer('Boot Code Regression', fRegression)
