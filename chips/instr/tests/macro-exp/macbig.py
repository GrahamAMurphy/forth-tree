import Sequencer
from core.#core import CMD		# CRISM
from common.core import CMD		# LORRI

TOTALBLK = 1780
FREEBLK  = 1762

def fBigMacro(oSeq, junk):
  CMD('LOR_MAC_DEF', 100)		# define one big macro
  for i in range(FREEBLK-1):
    CMD('LOR_MEM_LOAD', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
  			16, 17, 18)
    CMD('LOR_CMD_NULL')
  CMD('LOR_MAC_ENDDEF')

def fManyMacros(oSeq, junk):
  blk = TOTALBLK/255
  for i in range(255):			# define many medium macros
    CMD('LOR_MAC_DEF', i)
    for j in range(blk-1):
      CMD('LOR_MEM_LOAD', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
  			16, 17, 18)
      CMD('LOR_CMD_NULL')
    CMD('LOR_MAC_ENDDEF')
  blk = TOTALBLK - 255*blk
  CMD('LOR_MAC_DEF', 255)		# use up rest of blocks
  for i in range(blk-1):
    CMD('LOR_MEM_LOAD', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
  			16, 17, 18)
    CMD('LOR_CMD_NULL')
  CMD('LOR_MAC_ENDDEF')

Sequencer.Sequencer('Define Big Macro', fBigMacro, (), 0)
Sequencer.Sequencer('Define Many Macros', fManyMacros, (), 0)
