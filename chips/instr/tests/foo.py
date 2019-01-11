from types import IntType

def CMD(name, *args):
  cmdstr = name + '('
  cont = ''
  for arg in args:
    if type(arg) is IntType:
      cmdstr += cont + '%d'%arg
    else:
      cmdstr += cont + arg
    cont = ', '
  cmdstr += ')'
  return cmdstr
