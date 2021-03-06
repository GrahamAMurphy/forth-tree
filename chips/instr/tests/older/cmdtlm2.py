from __main__ import *
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

  def fLoadFile(path, addr):
    core.loadFile.fLOAD_FILE(path, 'MDS', addr)

# ------------------------------------------------------------------------
# LORRI/PEPSSI-specific things here

if host == 'NH':
  from common.core import CMD

  fLoadFile = i_LORRI.lor_com_cmds.fLOR_LOAD_FILE

# ------------------------------------------------------------------------
# CRISM-specific things here

if host == 'MRO':
  from core.core import CMD

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
  CMD(instr + '_' + name, args)

# Packet checking

def fPacketSeen(oSeq, block, timeout):
  seen = 1
  try:
    oSeq.Wait(block, lambda:1, (), timeout)
  except Sequencer.TimeoutError:
    seen = 0
  return seen

def fCheckPacket(oSeq, block):
  if not fPacketSeen(oSeq, block, 10):
      Error('Expected packet not seen')

def fCheckNoPacket(oSeq, block):
  if fPacketSeen(oSeq, block, 10):
      Error('Unexpected packet seen')

def fCheckStatus(oSeq, f):
  if fPacketSeen(oSeq, bStatus, 10):
    if not f():
      Error('Bad status')
  else:
    Error('No status')

def fCheckMemCheck(oSeq, src, words, sum):
  if fPacketSeen(oSeq, bMemCheck, 10):
    if not ((bMemCheck.Address==src) and
		(bMemCheck.MemLength==words) and
		(bMemCheck.CRC_Value==sum)):
      Error('Bad memory checksum')
  else:
    Error('No memory checksum')

# ------------------------------------------------------------------------
# Test Sequencer

def fCmdTlm(oSeq):
  fSendCmd('STAT_INT', 1)		# status packet every second
  fSendCmd('MEM_WR_EN', 1)		# enable memory write
  fLoadFile('/jrh/zeroes.mem', 0x60000)	# load zeros
  oSeq.Sleep(20)
  while ierrors==0:			# do while no errors
    fSendCmd('MEM_READ', 0, 65535)	# maximum volume of memory dump packets
    fSendCmd('MEM_COPY', 0x60000, 0x70000, 0x4000)
    oSeq.Sleep(3)			# allow time for copy
    fSendCmd('STAT_CLR', 255)		# reset command counters
    fLoadFile('/jrh/counts.mem', 0x70000) # load test file
    oSeq.Sleep(15)
    fSendCmd('MEM_CHECK', 0x70000, 8192)# check load
    fCheckMemCheck(oSeq, 0x70000, 8192, 0xf808)
    fCheckStatus(oSeq, lambda: (bStatus.CmdExec==66) and
				(bStatus.CmdReject==0) and
				(bStatus.AlarmCount==0))
  print 'Error'

Sequencer.Sequencer('Command/Telemetry Load Test', fCmdTlm)
