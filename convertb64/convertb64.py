#!/opt/local/bin/python
# Convert between a binary touchOSC config file and a human-readable XML file
# Convert Base64 encodings of names and osc_cs to ascii
import sys
import xml.etree.ElementTree as ET
import base64
import zipfile
import argparse



def indent(elem, level=0):
  i = "\n" + level*"  "
  if len(elem):
    if not elem.text or not elem.text.strip():
      elem.text = i + "  "
    if not elem.tail or not elem.tail.strip():
      elem.tail = i
    for elem in elem:
      indent(elem, level+1)
    if not elem.tail or not elem.tail.strip():
      elem.tail = i
  else:
    if level and (not elem.tail or not elem.tail.strip()):
      elem.tail = i

parser = argparse.ArgumentParser(description='Convert touchOSC files.')
parser.add_argument('-d', '--decode',action='store_true',help='decode from binary to an ASCII xml file')
parser.add_argument('-e', '--encode',action='store_true',help='encode from an ASCII xml file to a binary touchOSC file')
#parser.add_argument('input',help='input file name')
#parser.add_argument('output',help='outfile file name')
args = parser.parse_args()
infd=sys.stdin
outfd=sys.stdout

if args.decode:
  with zipfile.ZipFile(infd,'r') as myzip:
      index=myzip.open('index.xml','rU')
      tree = ET.parse(index)
      root = tree.getroot()
      indent(root)
      for elem in ['control','tabpage']:
          for neighbor in root.iter(elem):
              a=neighbor.attrib
              for tag in ['name','text','osc_cs']:
                  if tag in a:
                      orig=a[tag]
                      cvt=base64.b64decode(orig)
                      #print tag,": ",orig,' -> ',cvt
                      a[tag]=cvt

  tree.write(outfd,encoding="UTF-8",xml_declaration=True)
elif args.encode:
  with zipfile.ZipFile(outfd,'w') as myzip:
    tree = ET.parse(infd)
    root = tree.getroot()
    indent(root)
    for elem in ['control','tabpage']:
        for neighbor in root.iter(elem):
            a=neighbor.attrib
            for tag in ['name','text','osc_cs']:
                if tag in a:
                    orig=a[tag]
                    cvt=base64.b64encode(orig)
                    #print tag,": ",orig,' -> ',cvt
                    a[tag]=cvt
    myzip.writestr('index.xml',ET.tostring(root,encoding="UTF-8"))
else:
    parser.print_help()

    



