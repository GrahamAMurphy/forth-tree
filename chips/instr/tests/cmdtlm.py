from __main__ import *
import Sequencer

# ------------------------------------------------------------------------
# Select instrument

# instr = 'MDS'; host = 'MESS'
# instr = 'LOR'; host = 'NH'
# instr = 'PEP'; host = 'NH'
# instr = 'VSL'; host = 'NH'
instr = 'ATAS'; host = 'NH'
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
  fSendCmd('MEM_WR_EN', 1)		# enable memory write
  while 1:				# do forever
    fSendCmd('STAT_INT', 1)		# status packet every second
    fSendCmd('MEM_READ', 0, 65535)	# maximum volume of memory dump packets
    fLoadFile('/jrh/messfor.mem', 0x60000) # load test file 1 during dump
    oSeq.Sleep(15)
    fSendCmd('MEM_CHECK', 0x60000, 6150)# check load
    fCheckMemCheck(oSeq, 0x60000, 6150, 0x8a02)
    fSendCmd('STAT_INT', 1)		# status packet every second
    fSendCmd('MEM_READ', 0, 65535)	# maximum volume of memory dump packets
    fLoadFile('/jrh/messfore.mem', 0x60000) # load test file 2
    oSeq.Sleep(15)
    fSendCmd('MEM_CHECK', 0x60000, 6155)# check load
    fCheckMemCheck(oSeq, 0x60000, 6155, 0x8269)

Sequencer.Sequencer('Command/Telemetry Load Test', fCmdTlm)
