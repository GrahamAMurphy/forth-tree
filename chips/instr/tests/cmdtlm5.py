from __main__ import *
from regresstools import *

# ------------------------------------------------------------------------
# Test Sequencer

bStatus = bAppStatus			# status packet

def fCmdTlm(oSeq):
  ResetError()
  fSendCmd('STAT_INT', 1)		# status packet every second
  fSendCmd('MEM_WR_EN', enable)	# enable memory write
  fLoadFile('/jrh/zeroes.mem', 0x50000)	# load zeros
  fSyncIdle(oSeq, 5)			# wait for command echos to stop
  while ErrorsSeen()==0:		# do while no errors
    fSendCmd('STAT_CLR', all)		# reset command counters
    fSendCmd('MEM_READ', 0, 65535)	# maximum volume of memory dump packets
    fSendCmd('MEM_COPY', 0x50000, 0x60000, 0x4000)
    fSendCmd('CMD_NULL')		# annoy with interrupts during copy
    fSendCmd('CMD_NULL')
    fSyncIdle(oSeq, 5)			# wait for command echos to stop
    fLoadFile('/jrh/counts.mem', 0x60000) # load test file
    fSyncIdle(oSeq, 5)			# wait for command echos to stop
    fSendCmd('MEM_CHECK', 0x60000, 8192)# check load
    fCheckMemCheck(oSeq, 0x60000, 8192, 0xf808)
    fCheckPacket(oSeq, bStatus, lambda:
		#(bStatus.CmdExec==66) and
		(bStatus.CmdReject==0) and
		(bStatus.AlarmCount==0))
  print 'Error'

TSequencer('Command/Telemetry Load Test', fCmdTlm)
