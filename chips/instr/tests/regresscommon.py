# (c) 2003 Johns Hopkins University / Applied Physics Laboratory
# by John R. Hayes
from __main__ import *
from regresstools import *

# ------------------------------------------------------------------------
# Boot vs. Application
# Note: don't try to move this to regresstools; it won't work.

# Things that are different:
bStatus = bAppStatus		# status packet
undefcmd = 'ROM_BOOT'		# undefined command

def fPrepApp():
  global bStatus, undefcmd
  bStatus = bAppStatus
  undefcmd = 'ROM_BOOT'

def fPrepBoot():
  global bStatus, undefcmd
  bStatus = bBootStatus
  undefcmd = 'MAC_RESTORE'

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
  fSendCmd('STAT_CLR', all)
  fClrCmdExec(); fClrCmdReject(); fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [255])
  fCheckCmdCounts(oSeq)

def fBasicCommand(oSeq):
  # Precondition: status interval set to a high rate.
  fSyncCmd()
  C('---Basic command functionality---')
  C('---NULL command---')				# 1.1
  fSendCmd('CMD_NULL')
  fGoodCmd()
  fCheckEcho(oSeq, 'CMD_NULL', 0, [])
  fCheckCmdCounts(oSeq)
  C('---WRAPped NULL command---')			# 1.2
  if host=='NH':			# LORRI/PEPSSI-specific
    fSendCmd('CMD_WRAP', 0, dOpcodes['CMD_NULL'], 0, 0)
  elif host=='MESS':			# MESSENGER-specific
    fSendCmd('CMD_WRAP', dDests[instr], dOpcodes['CMD_NULL'], 0, 0)
  else:					# default
    fSendCmd('CMD_WRAP', dOpcodes['CMD_NULL'])
  fGoodCmd()
  fCheckEcho(oSeq, 'CMD_NULL', 0, [])
  fCheckCmdCounts(oSeq)
  C('---undefined command---')				# 1.3
  fSendCmd(undefcmd)
  fBadCmd()
#  if (host=='MRO') or \
#	(host=='Juno') or (host=='MMS') or \
#	(host=='BepiColombo'):
#    fCheckEcho(oSeq, undefcmd, 2, [0,0]) # some systems echo 2-byte pad
#  else:
#    fCheckEcho(oSeq, undefcmd, 2, [])
  fCheckEcho(oSeq, undefcmd, 2)
  fCheckCmdCounts(oSeq)
  C('---Check STAT_INT command with different intervals---') # 1.4
  for interval in [1, 2, 3, 10]:
    C('\tinterval = %d' % interval)
    fSendCmd('STAT_INT', interval)
    fGoodCmd()
    fCheckEcho(oSeq, 'STAT_INT', 0, [0,interval])
    fCheckCmdCounts(oSeq)
    fCheckSeq(oSeq, bStatus, 4, interval)
    fCheckPacket(oSeq, bStatus, lambda: (bStatus.StatusInt==interval), interval+10)
  C('---STAT_INT(0) turns off status---')		# 1.5
  fSendCmd('STAT_INT', 0)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_INT', 0, [0,0])
  oSeq.Sleep(2)				# wait for status to be off
  fCheckNoPacket(oSeq, bStatus)
  C('---Use STAT_INT(1) for rest of test---')		# 1.6
  fSendCmd('STAT_INT', 1)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_INT', 0, [0,1])
  fCheckCmdCounts(oSeq)
  C('---STAT_CLR(0) clears commands executed---')	# 1.7
  fSendCmd('STAT_CLR', cmdexec)
  fClrCmdExec(); fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [0])
  fCheckCmdCounts(oSeq)
  C('---STAT_CLR(1) clears commands rejected---')	# 1.8
  fSendCmd('STAT_CLR', cmdrej)
  fClrCmdReject(); fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [1])
  fCheckCmdCounts(oSeq)
  C('---STAT_CLR(255) clears all---')			# 1.9
  fSendCmd('STAT_CLR', all)
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
  fSyncCmd()
  C('---MEM_WR_EN to disable memory writes---')		# 2.1
  fSendCmd('MEM_WR_EN', disable)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_WR_EN', 0, [0x00])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MemWrite==0))
  fCheckCmdCounts(oSeq)
  C('---STAT_MEM sent to monitor test memory location---') # 2.2
  fSendCmd('STAT_MEM', tstpg<<16)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_MEM', 0, [0x00,tstpg,0x00,0x00])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.WatchMemId==tstpg) and
  				(bStatus.WatchAddr==0x0000))
  fCheckCmdCounts(oSeq)
  C('---MEM_LOAD should be rejected; write disabled---') # 2.3
  fSendCmd('MEM_LOAD', tstpg<<16, 1, 2, 3, 4)
  fBadCmd()
  fCheckEcho(oSeq, 'MEM_LOAD', 0x0a,
		fMemLoadEcho(tstpg<<16, 4, 1, 2))
  fCheckCmdCounts(oSeq)
  C('---MEM_COPY should be rejected; write disabled---') # 2.4
  fSendCmd('MEM_COPY', (tstpg<<16) + 2, tstpg<<16, 2)
  fBadCmd()
  fCheckEcho(oSeq, 'MEM_COPY', 0x0a,
		[0x00, tstpg, 0x00, 0x02, 0x00, tstpg, 0x00, 0x00, 0x00, 0x02])
  fCheckCmdCounts(oSeq)
  C('---MEM_WR_EN to enable memory writes---')		# 2.5
  fSendCmd('MEM_WR_EN', enable)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_WR_EN', 0, [0x01])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MemWrite==1))
  fCheckCmdCounts(oSeq)
  C('---MEM_LOAD should be accepted now---')		# 2.6
  fSendCmd('MEM_LOAD', tstpg<<16, 1, 2, 3, 4)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_LOAD', 0x00,
		fMemLoadEcho(tstpg<<16, 4, 1, 2))
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.WatchData[0]==0x01) and
  				(bStatus.WatchData[1]==0x02))
  fCheckCmdCounts(oSeq)
  C('---MEM_COPY should be accepted now---')		# 2.7
  fSendCmd('MEM_COPY', (tstpg<<16) + 2, tstpg<<16, 2)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_COPY', 0x00,
		[0x00, tstpg, 0x00, 0x02, 0x00, tstpg, 0x00, 0x00, 0x00, 0x02])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.WatchData[0]==0x03) and
  				(bStatus.WatchData[1]==0x04))
  fCheckCmdCounts(oSeq)
  C('---Load a file---')				# 2.8
  fLoadFile(testname, tstpg<<16)
  fSyncIdle(oSeq, 4)			# wait for command echos to stop
  fSyncCmd()				# sync up command counters
  C('---MEM_CHECK used to verify successful load---')	# 2.9
  fSendCmd('MEM_CHECK', tstpg<<16, testlen)
  fGoodCmd()
  fCheckMemCheck(oSeq, tstpg<<16, testlen, testsum)
  #fCheckEcho(oSeq, 'MEM_CHECK', 0x00,	# there is a race between echo and check
  #		[0x00, tstpg, 0x00, 0x00, (testlen>>8)&0xff, testlen&0xff])
  fSyncIdle(oSeq, 4)			# so, resync with echoes
  fCheckCmdCounts(oSeq)
  C('---MEM_READ used to peek at loaded data---')	# 2.10
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_READ', tstpg<<16, 16, 0)
  else:
    fSendCmd('MEM_READ', tstpg<<16, 16)
  fGoodCmd()
# fCheckEcho(oSeq, 'MEM_READ', 0x00,	# there is a race between echo and read
#		fMemReadEcho(tstpg<<16, 16))
  fCheckMemRead(oSeq, tstpg<<16, tstpg<<16, 16,
  			[0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f])
  fCheckCmdCounts(oSeq)
  C('---MEM_READ used to dump entire page---')		# 2.11
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_READ', 0x00000, 0xffff, 0)
  else:
    fSendCmd('MEM_READ', 0x00000, 0xffff)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_READ', 0x00,
		fMemReadEcho(0x00000, 0xffff))
  fCheckMemRead(oSeq, 0x00000, 0x01000, 256, [])
  fCheckSeq(oSeq, bMemRead, 10, 1)
  fCheckCmdCounts(oSeq)
  C('---MEM_READ aborts last dump, starts new dump---')	# 2.12
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_READ', 0x10000, 0xffff, 0)
  else:
    fSendCmd('MEM_READ', 0x10000, 0xffff)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_READ', 0x00,
		fMemReadEcho(0x10000, 0xffff))
  oSeq.Sleep(3)
  fCheckMemRead(oSeq, 0x10000, 0x11000, 256, [])
  fCheckSeq(oSeq, bMemRead, 10, 1)
  fCheckCmdCounts(oSeq)
  if host=='NH':			# LORRI/PEPSSI-specific
    C('---Test NH dump packet handshake---')		# 2.12.1
    fSyncTime(oSeq, 100, bStatus)	# rewind time so covered by sim file
    fCheckSeq(oSeq, bMemRead, 10, 2)	# should be every 2 seconds
  elif host=='MESS':			# MESSENGER-specific
    C('---Test MESSENGER dump packet rate control---')
    C('---MEM_READ_INT(2) produces packets every 2 seconds---') # 2.12.1
    fSendCmd('MEM_READ_INT', 2)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x02])
    oSeq.Sleep(3)
    fCheckSeq(oSeq, bMemRead, 10, 2)	# should be every 2 seconds
    fCheckCmdCounts(oSeq)
    C('---MEM_READ_INT(0) turns off packet production---') # 2.12.2
    fSendCmd('MEM_READ_INT', 0)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x00])
    oSeq.Sleep(3)
    fCheckNoPacket(oSeq, bMemRead)
    fCheckCmdCounts(oSeq)
    C('---MEM_READ_INT(1) produces packets every second---') # 2.12.3
    fSendCmd('MEM_READ_INT', 1)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x01])
    oSeq.Sleep(3)
    fCheckSeq(oSeq, bMemRead, 10, 1)	# should be every second
    fCheckCmdCounts(oSeq)
  C('---MEM_READ_ABRT aborts memory dump---')		# 2.13
  fSendCmd('MEM_READ_ABRT')
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_READ_ABRT', 0x00, [])
  oSeq.Sleep(2)				# wait for dump to stop
  fCheckNoPacket(oSeq, bMemRead)
  fCheckCmdCounts(oSeq)
  C('---STAT_CLR(4) clears telemetry volume---')	# 2.14
  fSendCmd('STAT_CLR', telvol)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [4])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.TlmVolume==0))
  fCheckCmdCounts(oSeq)
  C('---MEM_READ used to produce known telemetry volume---') # 2.15
  # TBD/BUG: assumes no other telemetry source; in LORRI, image descriptors flow
  fSendCmd('STAT_INT', 0)		# turn off status for now
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_INT', 0, [0,0])
  fSendCmd('STAT_CLR', telvol)		# zero volume again
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [4])
  if host=='MESS':			# MESSENGER GSEOS requires 'type'
    fSendCmd('MEM_READ', tstpg<<16, 4096, 0)
  else:
    fSendCmd('MEM_READ', tstpg<<16, 4096)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_READ', 0x00,
		fMemReadEcho(tstpg<<16, 4096))
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
  C('---CMD_NULL(APPEND) rejected, no macro being compiled---') # 3.1.1
  fSetMacroMode("APPEND")
  fSendCmd('CMD_NULL')
  fBadCmd()
  fCheckEcho(oSeq, 'CMD_NULL', 6, [])
  fCheckCmdCounts(oSeq)
  fSetMacroMode("EXE")
  C('---MAC_DEF(100) starts macro definitions---')	# 3.1.2
  fSendCmd('MAC_DEF', 100)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 0, [100])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacLearn==1))
  C('---MAC_DEF(101, EXE) should be rejected---')	# 3.1.3
  fSetMacroMode("EXE")
  fSendCmd('MAC_DEF', 101)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_DEF', 6, [101])
  fCheckCmdCounts(oSeq)
  C('---MAC_RUN(100) should fail because macro definition is not complete---')# 3.1.4
  fSetMacroMode("EXE")
  fSendCmd('MAC_RUN', 100)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_RUN', 3, [100])
  fCheckCmdCounts(oSeq)
  # fSetMacroMode("APPEND")
  C('---MAC_ENDDEF() ends macro definition---')		# 3.1.5
  fSendCmd('MAC_ENDDEF')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: ((bStatus.MacLearn==0) and
				bStatus.MacroBlocks==iMacBlocks-1))
  C('---MAC_RUN(100) runs macro---')			# 3.1.6
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 100)
  fGoodCmd()
  #oSeq.Sleep(6)
  fSyncIdle(oSeq, 4)
  fCheckEchoSeq([
	(dOpcodes['MAC_RUN'], 0, 0),
	(dOpcodes['MAC_END'], 1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==1) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==100))
  C('---MAC_ENDDEF() should be rejected if no macro being defined---') # 3.1.7
  fSendCmd('MAC_ENDDEF')
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_ENDDEF', 6, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_END() should be rejected if used outside of a macro---') # 3.1.8
  fSendCmd('MAC_END')
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_END', 5, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_NEST(100) should be rejected if used outside of a macro---') # 3.1.9
  fSendCmd('MAC_NEST', 100)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_NEST', 5, [100])
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(101), MAC_NEST(99), MAC_ENDDEF() can be defined') # 3.1.10
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
  C('---MAC_RUN(101) runs, but MAC_NEST(99) is rejected---') # 3.1.11
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 101)
  fGoodCmd()
  #oSeq.Sleep(6)
  fSyncIdle(oSeq, 4)
  fCheckEchoSeq([
	(dOpcodes['MAC_RUN'],  0, 0),
	(dOpcodes['MAC_NEST'], 1, 3),
	(dOpcodes['MAC_END'],  1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==2) and
				(bStatus.MacCmdReject==1) and
				(bStatus.MacId==101))
  C('---MAC_DEF(101), MAC_NEST(100), MAC_ENDDEF() redefines macro 101') # 3.1.12
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
  C('---MAC_RUN(101) runs successfully---')		# 3.1.13
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 101)
  fGoodCmd()
  #oSeq.Sleep(6)
  fSyncIdle(oSeq, 4)
  fCheckEchoSeq([
	(dOpcodes['MAC_RUN'],  0, 0),
	(dOpcodes['MAC_NEST'], 1, 0),
	(dOpcodes['MAC_END'],  1, 0),
	(dOpcodes['MAC_END'],  1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==5) and
				(bStatus.MacCmdReject==1) and
				(bStatus.MacId==101))
  C('---MAC_RUN_SILENT(101) runs silently---')		# 3.1.14
  fStartEchoSeq()
  fSendCmd('MAC_RUN_SILENT', 101)
  fGoodCmd()
  #oSeq.Sleep(6)
  fSyncIdle(oSeq, 4)
  fCheckEchoSeq([
	(dOpcodes['MAC_RUN_SILENT'],  0, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==5) and
				(bStatus.MacCmdReject==1) and
				(bStatus.MacId==101))
  # some cannot send large commands
  if (host!='RBSP') and (host!='MMS') and (host!='BepiColombo'):
    C('---MAC_DEF(101) starts macro definition---')	# 3.1.15
    fSendCmd('MAC_DEF', 101)
    fGoodCmd()
    fCheckEcho(oSeq, 'MAC_DEF', 0, [101])
    fCheckCmdCounts(oSeq)
    C('---MEM_LOAD large command accepted---')		# 3.1.16
    fSendCmd('MEM_LOAD', tstpg<<16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_LOAD', 1,
		fMemLoadEcho(tstpg<<16, 240, 0, 0))
    fCheckCmdCounts(oSeq)
    C('---MEM_LOAD larger command rejected---')		# 3.1.17
    fSendCmd('MEM_LOAD', tstpg<<16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0)
    fBadCmd()
    fCheckEcho(oSeq, 'MEM_LOAD', 6,
		fMemLoadEcho(tstpg<<16, 248, 0, 0))
    fCheckCmdCounts(oSeq)
    C('---MAC_ENDDEF() ends macro definition---')	# 3.1.18
    fSendCmd('MAC_ENDDEF')
    fGoodCmd()
    fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
    fCheckCmdCounts(oSeq)
  C('---STAT_CLR(2) clears macro command execute counter---') # 3.1.19
  fSendCmd('STAT_CLR', maccmdexec)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [2])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==0) and
				(bStatus.MacCmdReject==1))
  fCheckCmdCounts(oSeq)
  C('---STAT_CLR(3) clears macro command reject counter---') # 3.1.20
  fSendCmd('STAT_CLR', maccmdrej)
  fGoodCmd()
  fCheckEcho(oSeq, 'STAT_CLR', 0, [3])
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==0) and
				(bStatus.MacCmdReject==0))
  fCheckCmdCounts(oSeq)

def fMacroTools(oSeq):
  # Precondition: status interval set to a high rate.
  C('---Macros: delay, pause, halt, etc.---')
  fClrCounts(oSeq)			# reset command counters
  C('---MAC_DELAY(10) should be rejected if used outside macro definition---') # 3.2.1
  fSendCmd('MAC_DELAY', 10)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_DELAY', 5, [0,10])
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(102), MAC_DELAY(10), MAC_ENDDEF() defined---') # 3.2.2
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
  C('---MAC_RUN(102) runs successfully until DELAY---')	# 3.2.3
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 102)
  fGoodCmd()
  #oSeq.Sleep(6)				# wait for first part of macro to run
  fSyncIdle(oSeq, 4)			# wait for first part of macro to run
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_RUN'],   0, 0),
	(dOpcodes['MAC_DELAY'], 1, 0)])
  C('---MAC_RUN(102) completes successfully---')	# 3.2.4
  fStartEchoSeq()
  oSeq.Sleep(10)			# wait for rest of macro to run
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_END'],   1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==2) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==102))
  C('---MAC_PAUSE() should be rejected if used outside macro definition---') # 3.2.5
  fSendCmd('MAC_PAUSE', 0)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_PAUSE', 5, [0,0,0,0])
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(102), MAC_PAUSE(), MAC_ENDDEF() defined---') # 3.2.6
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
  C('---MAC_RUN(102) runs successfully until PAUSE---')	# 3.2.7
  fSyncTime(oSeq, 100, bStatus)		# rewind time
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 102)
  fGoodCmd()
  #oSeq.Sleep(6)				# wait for first part of macro to run
  fSyncIdle(oSeq, 4)			# wait for first part of macro to run
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_RUN'],   0, 0),
	(dOpcodes['MAC_PAUSE'], 1, 0)])
  C('---MAC_RUN(102) completes successfully---')	# 3.2.8
  fStartEchoSeq()
  fSyncTime(oSeq, 0x1234, bStatus)	# move time forward
  #oSeq.Sleep(2)				#   wait for rest of macro to run
  fSyncIdle(oSeq, 4)			#   wait for rest of macro to run
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_END'],   1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==4) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==102))
  C('---MAC_HALT(103) should be rejected because macro is undefined---') # 3.2.9
  fSendCmd('MAC_HALT', 103)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_HALT', 3, [103])
  fCheckCmdCounts(oSeq)
  C('---MAC_HALT(102) should be rejected because macro is not running---') # 3.2.10
  fSendCmd('MAC_HALT', 102)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_HALT', 7, [102])
  fCheckCmdCounts(oSeq)
  C('---Reset time, then MAC_RUN(102).  MAC_HALT(102) should kill macro---') # 3.2.11
  fSyncTime(oSeq, 100, bStatus);	# move time back
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 102)
  fGoodCmd()
  #oSeq.Sleep(3)				# wait for first part of macro to run
  fSyncIdle(oSeq, 4)			# wait for first part of macro to run
  fSendCmd('MAC_HALT', 102)		#   kill macro
  fGoodCmd()
  #oSeq.Sleep(4+2)			# wait for echos to arrive
  fSyncIdle(oSeq, 4)			# wait for echos to arrive
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_RUN'],   0, 0),
	(dOpcodes['MAC_PAUSE'], 1, 0),
	(dOpcodes['MAC_HALT'],  0, 0)])
  fStartEchoSeq()
  fSyncTime(oSeq, 0x1234, bStatus)	# move time forward
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
  C('---MAC_LOOP_BEGIN() should be rejected if used outside macro defn.---') # 3.3.1
  fSendCmd('MAC_LOOP_BEGIN', 10)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_LOOP_BEGIN', 5, [0,10])
  fCheckCmdCounts(oSeq)
  C('---MAC_LOOP_END() should be rejected if used outside macro defn.---') # 3.3.2
  fSendCmd('MAC_LOOP_END')
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_LOOP_END', 5, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(103), MAC_LOOP_BEGIN(5), MAC_LOOP_END(), and MAC_ENDDEF()---') # 3.3.3
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
  C('---MAC_RUN(103) runs successfully---')		# 3.3.4
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 103)
  fGoodCmd()
  #oSeq.Sleep(6)
  fSyncIdle(oSeq, 4)
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
  C('---MAC_DEF(103), MAC_LOOP_BEGIN(63), MAC_RUN(102),') # 3.3.5
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
  C('---Reset time.  MAC_RUN(103) forks 63 macros; all PAUSE---') # 3.3.6
  fSyncTime(oSeq, 100, bStatus)		# rewind time
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 103)
  fGoodCmd()
  #oSeq.Sleep(12)
  fSyncIdle(oSeq, 4)
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
  C('---Advance time; PAUSEd macros complete---')	# 3.3.7
  fStartEchoSeq()
  fSyncTime(oSeq, 0x1234, bStatus)	# move time forward
  #oSeq.Sleep(10)
  fSyncIdle(oSeq, 4)
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
  C('---Reset time.  MAC_RUN(103) forks 63 macros; all PAUSE---') # 3.3.8
  fSyncTime(oSeq, 100, bStatus)		# rewind time
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 103)
  fGoodCmd()
  #oSeq.Sleep(12)
  fSyncIdle(oSeq, 4)
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
  C('---MAC_RUN(102) runs until PAUSE; 64 macros running---') # 3.3.9
  fStartEchoSeq()
  fSendCmd('MAC_RUN', 102)
  fGoodCmd()
  #oSeq.Sleep(6)				# wait for first part of macro to run
  fSyncIdle(oSeq, 4)			# wait for first part of macro to run
  fCheckEchoSeq([			#   and check echos
	(dOpcodes['MAC_RUN'],   0, 0),
	(dOpcodes['MAC_PAUSE'], 1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==197) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==102))
  C('---MAC_RUN(102) should fail; out of macro contexts---') # 3.3.10
  fSendCmd('MAC_RUN', 102)
  fBadCmd()
  fCheckEcho(oSeq, 'MAC_RUN', 4, [102])
  fCheckCmdCounts(oSeq)
  C('---Advance time; all 64 PAUSEd macros complete---') # 3.3.11
  fStartEchoSeq()
  fSyncTime(oSeq, 0x1234, bStatus)	# move time forward
  #oSeq.Sleep(10)
  fSyncIdle(oSeq, 4)
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
  C('---MAC_READ(200,200) of undefined macro produces no packets---') # 3.4.1
  fSendCmd('MAC_READ', 200, 200)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_READ', 0, [200,200])
  fCheckNoPacket(oSeq, bMacRead)
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(200,200) of undefined macro produces default checksum---') # 3.4.2
  fSendCmd('MAC_CHECK', 200, 200)
  fGoodCmd()
  fCheckMacCheck(oSeq, 200, 200, [0xffff])
  #fCheckEcho(oSeq, 'MAC_CHECK', 0, [200,200])	# race between echo and check
  fSyncIdle(oSeq, 4)				# so, resync with echoes
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(200), CMD_NULL(), MAC_ENDDEF() defined---') # 3.4.3
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
  C('---MAC_READ(200,200) produces macro read packet---') # 3.4.4
  fSendCmd('MAC_READ', 200, 200)
  fGoodCmd()
  #fCheckEcho(oSeq, 'MAC_READ', 0, [200,200])	# race between echo and read
  fCheckMacRead(oSeq, 200, 0, m200)
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(200,200) produces checksum---') # 3.4.5
  fSendCmd('MAC_CHECK', 200, 200)
  fGoodCmd()
  fCheckMacCheck(oSeq, 200, 200, [fMacChecksum(m200)])
  #fCheckEcho(oSeq, 'MAC_CHECK', 0, [200,200])	# race between echo and check
  fSyncIdle(oSeq, 4)				# so, resync with echoes
  fCheckCmdCounts(oSeq)
  C('---MAC_READ(200,209) produces macro read packet, skips undefined---') # 3.4.6
  fSendCmd('MAC_READ', 200, 209)
  fGoodCmd()
  fCheckMacRead(oSeq, 200, 0, m200)
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(200,209) produces checksums, default for undefined---') # 3.4.7
  fSendCmd('MAC_CHECK', 200, 209)
  fGoodCmd()
  fCheckMacCheck(oSeq, 200, 209, [fMacChecksum(m200)]+[0xffff]*9)
  #fCheckEcho(oSeq, 'MAC_CHECK', 0, [200,209])	# race between echo and check
  fSyncIdle(oSeq, 4)				# so, resync with echoes
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(201), 10*CMD_NULL(), MAC_ENDDEF() defines multiblock macro---') # 3.4.8
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
  C('---MAC_READ(200,209) produces macro read packets for 200 and 201---') # 3.4.9
  fSendCmd('MAC_READ', 200, 209)
  fGoodCmd()
  fCheckMacRead(oSeq, 200, 0, m200)
  fCheckMacRead(oSeq, 201, 0, m201[0:32])
  fCheckMacRead(oSeq, 201, 1, m201[32:64])
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(200,209) produces checksums, default for undefined---') # 3.4.10
  fSendCmd('MAC_CHECK', 200, 209)
  fGoodCmd()
  fCheckMacCheck(oSeq, 200, 209,
		[fMacChecksum(m200),fMacChecksum(m201)]+[0xffff]*8)
  #fCheckEcho(oSeq, 'MAC_CHECK', 0, [200,209])	# race between echo and check
  fSyncIdle(oSeq, 4)				# so, resync with echoes
  fCheckCmdCounts(oSeq)
  C('---MAC_SAVE() command---')				# 3.4.11
  fSendCmd('MAC_SAVE')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_SAVE', 0, [], 20)	# long command on some systems
  fCheckCmdCounts(oSeq)
  C('---MAC_RESTORE after power cycle---')		# 3.4.12
  oSeq.MessageBoxModeless('Power cycle and reboot application.')
  fSyncCmd()				# sync up command counters
  fSendCmd('MAC_RESTORE')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_RESTORE', 0, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(200,209) produces checksums for restored macros---') # 3.4.13
  fSendCmd('MAC_CHECK', 200, 209)
  fGoodCmd()
  fCheckMacCheck(oSeq, 200, 209,
		[fMacChecksum(m200),fMacChecksum(m201)]+[0xffff]*8)
  #fCheckEcho(oSeq, 'MAC_CHECK', 0, [200,209])	# race between echo and check
  fSyncIdle(oSeq, 4)				# so, resync with echoes
  fCheckCmdCounts(oSeq)
  C('---MAC_DEF(*), MAC_ENDDEF() defines all macros---') # 3.4.14
  for i in range(256):
    fSendCmd('MAC_DEF', i)
    fGoodCmd()
    fCheckEcho(oSeq, 'MAC_DEF', 0, [i])
    fSendCmd('MAC_ENDDEF')
    fGoodCmd()
    fCheckEcho(oSeq, 'MAC_ENDDEF', 0, [])
  fCheckCmdCounts(oSeq)
  mall = [2, 0, dOpcodes['MAC_END']]
  C('---MAC_READ(0,255) used to dump all macros---')	# 3.4.15
  fSendCmd('MAC_READ', 0, 255)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_READ', 0x00, [0,255])
  fCheckSeq(oSeq, bMacRead, 10, 1)
  fCheckCmdCounts(oSeq)
  if host=='NH':			# LORRI/PEPSSI-specific
    C('---Test NH dump packet handshake---')		# 3.4.15.1
    fSyncTime(oSeq, 100, bStatus)	# rewind time so covered by sim file
    fCheckSeq(oSeq, bMacRead, 10, 2)	# should be every 2 seconds
  elif host=='MESS':			# MESSENGER-specific
    C('---Test MESSENGER dump packet rate control---')
    C('---MEM_READ_INT(2) produces packets every 2 seconds---')	# 3.4.15.1
    fSendCmd('MEM_READ_INT', 2)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x02])
    oSeq.Sleep(3)
    fCheckSeq(oSeq, bMacRead, 10, 2)	# should be every 2 seconds
    fCheckCmdCounts(oSeq)
    C('---MEM_READ_INT(0) turns off packet production---') # 3.4.15.2
    fSendCmd('MEM_READ_INT', 0)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x00])
    oSeq.Sleep(3)
    fCheckNoPacket(oSeq, bMacRead)
    fCheckCmdCounts(oSeq)
    C('---MEM_READ_INT(1) produces packets every second---') # 3.4.15.3
    fSendCmd('MEM_READ_INT', 1)
    fGoodCmd()
    fCheckEcho(oSeq, 'MEM_READ_INT', 0x00, [0x00, 0x01])
    oSeq.Sleep(3)
    fCheckSeq(oSeq, bMacRead, 10, 1)	# should be every second
    fCheckCmdCounts(oSeq)
  C('---MAC_READ(1,0) aborts macro dump---')	# 3.4.16 undocumented feature
  fSendCmd('MAC_READ', 1, 0)
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_READ', 0x00, [1,0])
  oSeq.Sleep(2)				# wait for dump to stop
  fCheckNoPacket(oSeq, bMacRead)
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(0,255) used to check all macros---')	# 3.4.17
  fSendCmd('MAC_CHECK', 0, 255)
  fGoodCmd()
  fCheckMacCheck(oSeq, 0, 255, [fMacChecksum(mall)]*256)
  #fCheckEcho(oSeq, 'MAC_CHECK', 0, [0,255])	# race between echo and check
  fSyncIdle(oSeq, 4)				# so, resync with echoes
  fCheckCmdCounts(oSeq)
  C('---MAC_RESTORE() command---')			# 3.4.18
  fSendCmd('MAC_RESTORE')
  fGoodCmd()
  fCheckEcho(oSeq, 'MAC_RESTORE', 0, [])
  fCheckCmdCounts(oSeq)
  C('---MAC_CHECK(200,209) produces checksums for restored macros---') # 3.4.19
  fSendCmd('MAC_CHECK', 200, 209)
  fGoodCmd()
  fCheckMacCheck(oSeq, 200, 209,
		[fMacChecksum(m200),fMacChecksum(m201)]+[0xffff]*8)
  #fCheckEcho(oSeq, 'MAC_CHECK', 0, [200,209])	# race between echo and check
  fSyncIdle(oSeq, 4)				# so, resync with echoes
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
  # TBD: missing PWR_OFF check
  C('---Safing---')
  fClrCounts(oSeq)			# clear command counters
  C('---MAC_DEF(1), MAC_NEST(2), MAC_ENDDEF() replaces shutdown macro---')
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
  C('---MAC_DEF(2), MAC_ENDDEF() replaces safing macro---')
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
  #oSeq.Sleep(6)
  fSyncIdle(oSeq, 4)
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
  #oSeq.Sleep(6)
  fSyncIdle(oSeq, 4)
  fCheckEchoSeq([
	(dOpcodes['SHUTDOWN'], 0, 0),
	(dOpcodes['MAC_NEST'], 1, 0),
	(dOpcodes['MAC_END'],  1, 0),
	(dOpcodes['MAC_END'],  1, 0)])
  fCheckCmdCounts(oSeq)
  fCheckPacket(oSeq, bStatus, lambda: (bStatus.MacCmdExec==4) and
				(bStatus.MacCmdReject==0) and
				(bStatus.MacId==1))
  C('---SOFT_RESET() causes watchdog reset back to boot---')
  fSendCmd('SOFT_RESET')		# no command echo
  oSeq.Sleep(5)				# wait for watchdog ...
  fCheckPacket(oSeq, bBootStatus, lambda:1)

# ------------------------------------------------------------------------
# Program load and boot

# TBD: explicitly uses bBootStatus; ok?

def fBoot(oSeq):
  # Precondition: status interval set to a high rate.
  C('---Program load and boot---')
  #fClrCounts(oSeq)			# clear command counters
  fSyncCmd()
  C('---MEM_WR_EN to enable memory writes---')		# 4.1
  fSendCmd('MEM_WR_EN', enable)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_WR_EN', 0, [0x01])
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.MemWrite==1))
  fCheckCmdCounts(oSeq)
  C('---Load a file: %s---' % progname)			# 4.2
  fLoadFile(progname, 0)
  fSyncIdle(oSeq, 4)			# wait for command echos to stop
  fSyncCmd()				# sync up command counters
  C('---MEM_CHECK used to verify successful load---')	# 4.3
  fSendCmd('MEM_CHECK', 0, proglen)
  fGoodCmd()
  fCheckMemCheck(oSeq, 0, proglen, progsum)
# fCheckEcho(oSeq, 'MEM_CHECK', 0x00,	# there is a race between echo and check
#		[0x00, tstpg, 0x00, 0x00, 0x18, 0x06])
  fSyncIdle(oSeq, 4)			# so, resync with echoes
  fCheckCmdCounts(oSeq)
  C('---ROM_GO rejected, incorrect format---')		# 4.4
  fSendCmd('ROM_GO', 0)
  fBadCmd()
  fCheckEcho(oSeq, 'ROM_GO', 8, [0x00,0x00,0x00,0x00])
  fCheckCmdCounts(oSeq)
  C('---MEM_RUN used to start program---')		# 4.5
  fSendCmd('MEM_RUN', progstart)	# no echo
  oSeq.MessageBoxModeless('Confirm Forth application.  Reset.')
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.Cause==0))
  fSyncCmd()				# sync up command counters
  C('---MEM_WR_EN to enable memory writes---')		# 4.6
  fSendCmd('MEM_WR_EN', enable)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_WR_EN', 0, [0x01])
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.MemWrite==1))
  fCheckCmdCounts(oSeq)
  C('---Load a file: %s' % eeprogname)			# 4.7
  fLoadFile(eeprogname, 0)
  fSyncIdle(oSeq, 4)			# wait for command echos to stop
  fSyncCmd()				# sync up command counters
  C('---MEM_CHECK used to verify successful load---')	# 4.8
  fSendCmd('MEM_CHECK', 0, eeproglen)
  fGoodCmd()
  fCheckMemCheck(oSeq, 0, eeproglen, eeprogsum)
# fCheckEcho(oSeq, 'MEM_CHECK', 0x00,	# there is a race between echo and check
#		[0x00, tstpg, 0x00, 0x00, 0x18, 0x06])
  fSyncIdle(oSeq, 4)			# so, resync with echoes
  fCheckCmdCounts(oSeq)
  C('---MEM_COPY to save program to EEPROM---')		# 4.9
  fSendCmd('MEM_COPY', 0, eeprogpg<<16, 0x4000)
  fGoodCmd()
  fCheckEcho(oSeq, 'MEM_COPY', 0x00,
		[0x00, 0x00, 0x00, 0x00, 0x00, eeprogpg, 0x00, 0x00, 0x40, 0x00])
  fCheckCmdCounts(oSeq)
  C('---MEM_CHECK used to verify successful copy---')	# 4.10
  fSendCmd('MEM_CHECK', eeprogpg<<16, eeproglen)
  fGoodCmd()
  fCheckMemCheck(oSeq, eeprogpg<<16, eeproglen, eeprogsum)
# fCheckEcho(oSeq, 'MEM_CHECK', 0x00,	# there is a race between echo and check
#		[0x00, tstpg, 0x00, 0x00, 0x18, 0x06])
  fSyncIdle(oSeq, 4)			# so, resync with echoes
  fCheckCmdCounts(oSeq)
  C('---ROM_GO used to start saved program---')		# 4.11
  fSendCmd('ROM_GO', eeprogpg<<16)	# no echo
  oSeq.MessageBoxModeless('Confirm Forth application.  Trigger watchdog reset')
  fCheckPacket(oSeq, bBootStatus, lambda: (bBootStatus.Cause==1))
  fSyncCmd()				# sync up command counters
  if not (host=='MESS' or host=='NH' or host=='MRO'):	# available on newer systems
    C('---Watchdog reset triggers memory dump---')	# 4.12
    fCheckMemRead(oSeq, 0x00000, 0x0ffff, 256, [])
    fCheckSeq(oSeq, bMemRead, 10, 1)
  C('---ROM_BOOT used to start default program---')	# 4.13
  fSendCmd('ROM_BOOT')			# no echo
  oSeq.MessageBoxModeless('Confirm default application.')

# ------------------------------------------------------------------------
# Top level test

def fCommonRegression(oSeq):
# TBD: Preconditions:
  C('---Starting common regression test---')
  fPrepApp()
  ResetError()
  fSyncTime(oSeq, 1000, bStatus)
  fBasicCommand(oSeq)
  fMemory(oSeq)
  fMacros(oSeq)
#  if host!='MRO':			# not supported on CRISM
#    fSafing(oSeq)
  C('---Regression test done---')
  ErrorReport()

fDefTest('Common Code Regression', fCommonRegression)

# ------------------------------------------------------------------------
# Top level test

def fBootRegression(oSeq):
  # Precondition: /jrh/messfor.mem and /jrh/messfore.mem on host.
  # Precondition: if NH, 2sec.sim loaded into GSEOS.
  C('---Starting boot regression test---')
  fPrepBoot()
  ResetError()
  fSyncTime(oSeq, 1000, bStatus)
  fBasicCommand(oSeq)
  fMemory(oSeq)
  fBoot(oSeq)
  C('---Regression test done---')
  ErrorReport()

fDefTest('Boot Code Regression', fBootRegression)
