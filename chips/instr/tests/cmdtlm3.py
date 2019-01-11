from __main__ import *
import Sequencer

# ------------------------------------------------------------------------
# Select instrument

# instr = 'MDS'; host = 'MESS'
# instr = 'LOR'; host = 'NH'
# instr = 'PEP'; host = 'NH'
instr = 'CRM'; host = 'MRO'

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

def fCheckPacket(block, f):
  if not f():
    Error('Bad packet data')

# ------------------------------------------------------------------------
# Test Sequencer

# Note: this version runs open loop to avoid a race.

seqno = 0

def fCmdTlm(oSeq):
  global seqno
  fSendCmd('STAT_INT', 1)		# status packet every second
  fSendCmd('MEM_WR_EN', 1)		# enable memory write
  fSendCmd('MEM_CHECK', 0x60000, 2)	# generate a memory checksum packet
  fLoadFile('/jrh/zeroes.mem', 0x50000)	# load zeros
  oSeq.Sleep(20)
  seqno = bMemCheck.SeqCount		# get base sequence number
  while ierrors==0:			# do while no errors
    seqno = (seqno + 1) & 0x3fff	# predict next sequence number
    fSendCmd('MEM_READ', 0, 65535)	# maximum volume of memory dump packets
    fSendCmd('MEM_COPY', 0x50000, 0x60000, 0x4000)
    oSeq.Sleep(3)			# allow time for copy
    fSendCmd('STAT_CLR', 255)		# reset command counters
    fLoadFile('/jrh/counts.mem', 0x60000) # load test file
    oSeq.Sleep(15)
    fSendCmd('MEM_CHECK', 0x60000, 8192)# check load
    oSeq.Sleep(3)			# allow time for packets to arrive
    fCheckPacket(bMemCheck, lambda:	# confirm new packet and correct sum
		(bMemCheck.SeqCount==seqno) and
		(bMemCheck.CRC_Value==0xf808))
    fCheckPacket(bBootStatus, lambda:
		(bBootStatus.CmdExec==66) and
		(bBootStatus.CmdReject==0) and
		(bBootStatus.AlarmCount==0))
  print 'Error'

Sequencer.Sequencer('Command/Telemetry Load Test', fCmdTlm)
