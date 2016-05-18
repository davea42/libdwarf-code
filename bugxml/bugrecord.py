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

# Do to a single text line.
def xmlize(linea):
  outi =  []
  l = linea
  if l.find("<pre>") != -1:
     s2 = l + '\n'
     return s2
  if l.find("</pre>") != -1:
     s2 = l + '\n'
     return s2
  for c in l:
    if c == '<':
       outi += ["&lt;"]
    elif c == '>':
       outi += ["&gt;"]
    elif c == "&":
       outi += ["&amp;"]
    elif c == "'":
       outi += ["&apos;"]
    elif c == '"':
       outi += ["&quot;"]
    else:
       outi += [c]
  outi += ["\n"]
  s2 = ''.join(outi)
  return s2

def paraline(name,linea):
  out = ''
  if len(linea) <1:
    out = "<p>" + name + ":"+ "</p>"
    return out
  out = "<p>" + name + ": "
  out +=linea 
  out += "</p>"
  return out;

def paralines(name,lines):
  if len(lines) <1:
    out = "<p>" + name + ":"+ "</p>"
    return out
  out = "<p>" + name + ": "
  for lin in lines:
     f = xmlize(lin)
     out += f
  out += "</p>"
  return out;

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
    s5= ''.join(self._id)
    t = ''.join(['<h3 id="',s5,'">',self._id,'</h3>'])
    txt = [t]

    t = paraline("id",xmlize(self._id))
    txt += [t]
    t = paraline("cve",xmlize(self._cve))
    txt += [t]
    t = paraline("datereported",xmlize(self._datereported))
    txt += [t]
    t = paraline("reportedby",xmlize(self._reportedby))
    txt += [t]
 
    #MULTI
    t = paralines("vulnerability",self._vulnerability)
    txt += [t]


    t = paraline("product",xmlize(self._product))
    txt += [t]
    
    #MULTI
    t = paralines("description",self._description)
    txt += [t]

    t = paraline("datefixed",xmlize(self._datefixed))
    txt += [t]

    #MULTI
    t = paralines("references",self._references)
    txt += [t]

    t = paraline("gitfixid",xmlize(self._gitfixid))
    txt += [t]
    t = paraline("tarrelease",xmlize(self._tarrelease))
    txt += [t]

    t = '<p> <a href="#top">[top]</a> </p>'
    txt += [t]
    return txt

  def paraxml(self,start,main,term):
    # For single line xml remove the newline from the main text line.
    out = start
    l=main.strip()
    if len(l) > 0:
      out += l 
    out += term + "\n" 
    return out
  def paraxmlN(self,start,main,term):
    # For multi line xml leave newlines present.
    out = start
    for x in main:
      l=x.strip()
      t = xmlize(l);
      if len(t.strip()) > 0:
        out += t
    out += term + "\n"
    return out


  def generate_xml(self):
    txt=[]
    t = '<dwbug>'
    txt += [t]
     
    t = self.paraxml('<dwid>',xmlize(self._id),'</dwid>')
    txt += [t]
    t = self.paraxml('<cve>',xmlize(self._cve),'</cve>')
    txt += [t]

    t = self.paraxml('<datereported>',xmlize(self._datereported),'</datereported>')
    txt += [t];

    t = self.paraxml('<reportedby>',xmlize(self._reportedby),'</reportedby>')
    txt += [t];

    t = self.paraxml('<product>',xmlize(self._product),'</product>')
    txt += [t];

    
    #MULTI
    p = self._vulnerability
    t = self.paraxmlN("<vulnerability>",p,"</vulnerability>")
    txt += [t]


    #MULTI
    p = self._description
    t = self.paraxmlN("<description>",p,"</description>")
    txt += [t]


    t = self.paraxml('<datefixed>',xmlize(self._datefixed),'</datefixed>')
    txt += [t];

    #MULTI
    p = self._references    
    t = self.paraxmlN("<references>",p,"</references>")
    txt += [t]

    t = self.paraxml('<gitfixid>',xmlize(self._gitfixid),'</gitfixid>')
    txt += [t];
    t = self.paraxml('<tarrelease>',xmlize(self._tarrelease),'</tarrelease>')
    txt += [t];

    t = '</dwbug>'
    txt += [t];
    return txt
