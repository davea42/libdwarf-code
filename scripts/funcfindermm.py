#! /usr/bin/env python3
#funcfindermm.py

import os
import sys

#Reads libdwarf2p.1.mm and produces a concise list
#of the documented functions, sorted by function name.

# Run as
# funcfindermm.py 
# or
# funcfindermm.py --nonumbers


# \f(CWint dwarf_producer_init(

def looks_like_func(s):
  fwds = s.split("(")
  if len(fwds) < 2:
    #print("Not func",fwds)
    return "n","",""
  if fwds[1].startswith(")"):
    return "n","",""
  fwds2 = fwds[0].split(" ")
  if len(fwds2) < 2:
    #print("Not func",fwds)
    return "n","",""
  return "y",fwds2[1],fwds2[0]

def reader(path):
  try:
    file = open(path,"r")
  except IOError as  message:
    print("File could not be opened: ", message,file=sys.stderr)
    sys.exit(1)
  out = []
  iline = 0
  for line in file:
    iline = int(iline) + 1
    l2 = line.rstrip()
    wds = l2.split('W');
    #if wds[0] != ".H 3 ":
    if len(wds) < 2:
      continue
    if not wds[0].startswith("\\"):
      continue
    #print(wds[0])
    if wds[0] != "\\f(C":
      continue
    isf,func,ret = looks_like_func(wds[1])
    if isf == "y":
      out += [(func,ret,iline)] 
  file.close()
  return out
if __name__ == '__main__':
  showlinenum = "y"
  if len(sys.argv) > 1:
    if sys.argv[1] == "--nonumbers":
      showlinenum = "n"
    else:
      print("argument ", sys.argv[1],"ignored") 
  funcl = reader("libdwarf2p.1.mm");

  funcl.sort()
  for f in funcl:
    if showlinenum == "y":
      print(f)
    else:
      print(f[0],f[1])
    
