from random import random

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

