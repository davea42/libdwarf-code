#! /usr/bin/env python3
#funcfinderhdr.py

#Reads libdwarf.h.in and produces a concise list
#of the declared functions, sorted by function name.

# Run as
# funcfinderhdr.py 
# or
# funcfinderhdr.py --nonumbers

import os
import sys

def looks_like_func(s):
  fwds = s.split("(")
  if len(fwds) < 2:
    #print("Not func",fwds)
    return "n","","",""
  if fwds[1].startswith(")") == 1:
    return "n","","",""
  fwds2 = fwds[0].split(" ")
  if len(fwds2) < 2:
    #print("Not func",fwds)
    return "n","","",""
  if len(fwds2) == 2:
    return "y",fwds2[1],fwds2[0],""
  if len(fwds2) == 3:
    return "y",fwds2[2],fwds2[0],fwds2[1]
  return "n","","",""

def reader(path):
  try:
    file = open(path,"r")
  except IOError as  message:
    print("File could not be opened: ", message,file=sys.stderr)
    sys.exit(1)
  out = []
  iline = 0
  saveword = ""
  for line in file:
    iline = int(iline) + 1
    l2 = line.rstrip()
    #if wds[0] != ".H 3 ":
    #print(wds[0])
    isf,func,reta,retb = looks_like_func(l2)
    if isf == "y":
      out += [(func,reta,retb,iline)] 
  file.close()
  return out
if __name__ == '__main__':
  showlinenum = "y"
  if len(sys.argv) > 1:
    if sys.argv[1] == "--nonumbers":
      showlinenum = "n"
    else:
      print("argument ", sys.argv[1],"ignored") 

  full_list = reader("libdwarf.h.in");
  full_list.sort()
  for f in full_list:
    if showlinenum == "y":
      print(f)
    else:
      print(f[0],f[1])
    
