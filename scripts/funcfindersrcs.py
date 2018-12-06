#! /usr/bin/env python3
#funcfindersrcs.py

import os
import sys

#Reads the producer code source and produces a concise list
#of the implemented functions, sorted by function name.

# Run as
# funcfindersrcs.py 
# or
# funcfindersrcs.py --nonumbers


filelist =  [
"pro_alloc.c",
"pro_alloc.c",
"pro_arange.c",
"pro_die.c",
"pro_dnames.c",
"pro_encode_nm.c",
"pro_error.c",
"pro_expr.c",
"pro_finish.c",
"pro_forms.c",
"pro_frame.c",
"pro_funcs.c",
"pro_init.c",
"pro_line.c",
"pro_macinfo.c",
"pro_pubnames.c",
"pro_reloc.c",
"pro_reloc_stream.c",
"pro_reloc_symbolic.c",
"pro_section.c",
"pro_types.c",
"pro_vars.c",
"pro_weaks.c",
"dwarf_util.c",
"dwarf_util.c",
]


def looks_like_func(s):
  fwds = s.split("(")
  if len(fwds) < 2:
    #print("Not func",fwds)
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
  prevline = ''
  all_the_lines = file.read().splitlines()
  file.close()
  #print("No. Lines:",len(all_the_lines))
  for line in all_the_lines:
    l2 = line.rstrip()
    iline = int(iline) + 1
    if l2.startswith(" ") == 1:
      continue
    if l2.startswith("/") == 1:
      continue
    if l2.startswith("dwarf_") == 1:
      l3 = l2.split("(");
      if len(l3) < 2:
        printf(" dadebug odd-a ",l3);
        continue
      if l3[1].startswith(")") == 1:
        continue
      out += [(l3[0],prev,iline,path)] 
    else:
       prev=l2.strip()
  return out
if __name__ == '__main__':
  showlinenum = "y"
  if len(sys.argv) > 1:
    if sys.argv[1] == "--nonumbers":
      showlinenum = "n"
    else:
      print("argument ", sys.argv[1],"ignored") 
  funcl = []
  for path in filelist:
    funcl += reader(path)
  funcl.sort()
  for f in funcl:
    if showlinenum == "y":
      print(f)
    else:
      print(f[0],f[1])
    
