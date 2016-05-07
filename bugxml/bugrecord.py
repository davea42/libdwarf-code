#!/usr/bin/python
#  Copyright (c) 2016-2016 David Anderson.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of the example nor the
#    names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY David Anderson ''AS IS'' AND ANY
#  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL David Anderson BE LIABLE FOR ANY
#  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
#  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
#  OF SUCH DAMAGE.


import sys

def xmlize(s):
  out = []
  for c in s:
    if c == '<':
       out += ["&lt;"]
    elif c == '>':
       out += ["&gt;"]
    elif c == "&":
       out += ["&amp;"]
    elif c == "'":
       out += ["&apos;"]
    elif c == '"':
       out += ["&quot;"]
    else:
       out += [c]
  s2 = ''.join(out)
  return s2;

def para(name,str):
  if str ==  None:
      out = "<p>" + name + ":"+ "</p>"
  elif len(str) > 0:
      out = "<p>" + name + ":&nbsp" + str + "</p>"
  else:
      out = "<p>" + name + ":"+ "</p>"
  return out


class bugrecord:

  def __init__(self,dwid):
    self._id= dwid.strip()
    self._cve  = ''
    self._datereported  = ''
    self._reportedby  = ''
    self._vulnerability  = []
    self._product  = ''
    self._description  = []
    self._datefixed  = ''
    self._references  = []
    self._gitfixid  = ''
    self._tarrelease  = ''

  def setcve(self,pubid):
    if self._cve != '':
      print "Duplicate cve ",self._cve,pubid
      sys.exit(1)
    self._cve  = pubid.strip()
  def setdatereported(self,rep):
    if self._datereported != '':
      print "Duplicate datereported ",self._datereported,rep
      sys.exit(1)
    self._datereported  = rep.strip()
  def setreportedby(self,rep):
    if self._reportedby != '':
      print "Duplicate reportedby ",self._reportedby,rep
      sys.exit(1)
    self._reportedby  = rep.strip()
  def setvulnerability(self,vuln):
    if len(self._vulnerability) != 0:
      print "Duplicate vulnerability ",self._vulnerability,vuln
      sys.exit(1)
    self._vulnerability  = vuln
  def setproduct(self,p):
    if len(self._product) != 0:
      print "Duplicate product ",self._product,p
      sys.exit(1)
    self._product  = p.strip()
  def setdescription(self,d):
    if len(self._description) != 0:
      print "Duplicate description ",self._description,d
      sys.exit(1)
    self._description  = d
  def setdatefixed(self,d):
    if len(self._datefixed) != 0:
      print "Duplicate datefixed ",self._datefixed,d
      sys.exit(1)
    self._datefixed  = d.strip()
  def setreferences(self,r):
    if len(self._references) != 0:
      print "Duplicate references ",self._references,r
      sys.exit(1)
    self._references  = r
  def setgitfixid(self,g):
    if len(self._gitfixid) != 0:
      print "Duplicate gitfixid ",self._gitfixid,g
      sys.exit(1)
    self._gitfixid  = g.strip()
  def settarrelease(self,g):
    if len(self._tarrelease) != 0:
      print "Duplicate tarrelease ",self._tarrelease,g
      sys.exit(1)
    self._tarrelease  = g.strip()
  def plist(self,title,lines):
    if lines == None:
      print title
      return
    if len(lines) == 1:
      print title,lines[0]
      return
    print title
    for l in lines:
        print l

  def printbug(self):
    print ""
    print "id:",self._id
    print "cve:",self._cve
    print "datereported:",self._datereported
    print "reportedby:",self._reportedby
    self.plist("vulnerability:",self._vulnerability)
    print "product:",self._product
    self.plist("description:",self._description)
    print "datefixed:",self._datefixed
    self.plist("references:",self._references)
    print "gitfixid:",self._gitfixid
    print "tarrelease:",self._tarrelease

  def generate_html(self):
    t = ''.join(['<h3 id="',self._id,'">',self._id,'</h3>'])
    txt = [t]

    t = para("id",xmlize(self._id))
    txt += [t]
    t = para("cve",xmlize(self._cve))
    txt += [t]
    t = para("datereported",xmlize(self._datereported))
    txt += [t]
    t = para("reportedby",xmlize(self._reportedby))
    txt += [t]
 
    v = ''.join(self._vulnerability)    
    t = para("vulnerability",v)
    txt += [t]


    t = para("product",xmlize(self._product))
    txt += [t]
    
    t = para("product",xmlize(self._product))


    p = ''.join(self._description)    
    t = para("description",p)
    txt += [t]

    t = para("datefixed",xmlize(self._datefixed))
    txt += [t]

    p = ''.join(self._references)    
    t = para("references",p)
    txt += [t]

    #* references 
    t = para("gitfixid",xmlize(self._gitfixid))
    txt += [t]
    t = para("tarrelease",xmlize(self._tarrelease))
    txt += [t]

    t = '<p> <a href="#top">[top]</a> </p>'
    txt += [t]
    return txt

  def generate_xml(self):
    txt=[]
    t = '<dwbug>'
    txt += [t]
    t = ''.join(['<dwid>',xmlize(self._id),'</dwid>'])
    txt += [t]
    t = ''.join(['<cve>',xmlize(self._cve),'</cve>'])
    txt += [t]
    t = ''.join(['<datereported>',xmlize(self._datereported),'</datereported>'])
    txt += [t];
    t = ''.join(['<reportedby>',xmlize(self._reportedby),'</reportedby>'])
    txt += [t];
    #* vulnerability */



    t = ''.join(['<product>',xmlize(self._product),'</product>'])
    txt += [t];

    if len(self._vulnerability) > 0:
      p = ''.join(self._vulnerability)    
    else:
      p = ""
    t = ''.join(["<vulnerability>",xmlize(p),"</vulnerability>"])
    txt += [t]


    if len(self._description) > 0:
      p = ''.join(self._description)    
    else:
      p=""
    t = ''.join(["<description>",xmlize(p),"</description>"])
    txt += [t]


    t = ''.join(['<datefixed>',xmlize(self._datefixed),'</datefixed>'])
    txt += [t];

    if len(self._references) > 0:
      p = ''.join(self._references)    
    else:
      p = ""
    t = ''.join(["<references>",xmlize(p),"</references>"])
    txt += [t]

    t = ''.join(['<gitfixid>',xmlize(self._gitfixid),'</gitfixid>'])
    txt += [t];
    t = ''.join(['<tarrelease>',xmlize(self._tarrelease),'</tarrelease>'])
    txt += [t];

    t = '</dwbug>'
    txt += [t];
    return txt
