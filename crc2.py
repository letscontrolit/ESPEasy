
import sys
import binascii
import struct
from hashlib import md5
import os
MD5DUMMY      = "MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere..." #48 chars
FILENAMEDUMMY = "ThisIsTheDummyPlaceHolderForTheBinaryFilename64ByteLongFilenames" #64 chars

MemorySegmentStart,MemorySegmentEnd,MemoryContent=[],[],[]

##################################################################
# this subroutine shows the segments of a  part
##################################################################
def showSegments (fileContent,offset):
    global MemorySegmentStart, MemorySegmentEnd, MemoryContent
    header = struct.unpack("ii", fileContent[offset:offset+8])
    herestr =""
    herestr2 =""
    MemorySegmentStart.append(struct.pack("I",header[0]))
    MemorySegmentEnd.append(struct.pack("I",header[0]+header[1]))
    MemoryContent.append(fileContent[offset+8:offset+8+header[1]])
    if  fileContent.find( MD5DUMMY, offset+8, offset+8+header[1]) >0 :
        herestr= " <-- CRC is here."
    if  fileContent.find( FILENAMEDUMMY, offset+8, offset+8+header[1]) >0 :
        herestr2= " <-- filename is here."
    print ("SEGMENT "+ str(len(MemorySegmentStart)-1)+ ": memory position: " + hex(header[0])+" to " + hex(header[0]+header[1]) +  " length: " + hex(header[1])+herestr+herestr2)
    #print ("first byte positoin in file: " + hex( offset+8))
    #print ("last byte postion in file: " + hex(offset+8+header[1]-1))
    return (8+offset+ header[1]); # return start of next segment

##################################################################
# this subroutine shows the  parts of a binary file
##################################################################
def showParts(fileContent, offset):
    header = struct.unpack("BBBBi", fileContent[offset:offset+8])
    print ('\n\nBINARY PART\nSegments: ') + (hex(header[1]))
    nextpos =offset+8
    for x in range (0,header[1]):
        nextpos = showSegments(fileContent,nextpos)
    nextSegmentOffset = (fileContent.find("\xe9", nextpos))
    return nextSegmentOffset

##################################################################
# MAIN
##################################################################

#if len(sys.argv) !=2 :
#    print ("please give a filename")
#    k=input("press close to exit")
#    sys.exit(1)
FileName = sys.argv[1]
#FileName =  "C:/ArduinoPortable/sketchbooks/build/sketch_jan15a.ino.bin"
print( "\n\nReplacing dummy MD5 checksum in .bin file")

with open(FileName, mode='rb') as file: # b is important -> binary
    fileContent = file.read()

firmware_esp8266 = fileContent.find("ID_EASY_ESP8266") >= 0
firmware_esp32   = fileContent.find("ID_EASY_ESP32") >= 0

if firmware_esp8266:
  print("ESP8266 build")
if firmware_esp32:
  print("ESP32 build")

# TODO: showParts does not work for esp32
if firmware_esp8266:
  nextpos =0;
  while nextpos >=0:
        nextpos = showParts(fileContent,nextpos)

startArray,endArray,hashString = "","",""
includeStr = "hash includes segments:"
# memory sections:
# 0: bootloader (not readable)
# 1: program memory (SPI flash)
# 2: unknown but stable
# 3: RAM (initialized by bin file. Can be read but changes as you go :-( )
# 4: RAM

if firmware_esp8266:
  for i in (1,2 ):     # use only stable segments, must be 4 in total. We use 2.
    startArray =startArray + MemorySegmentStart[i]
    endArray =  endArray   + MemorySegmentEnd[i]
    hashString =hashString + MemoryContent[i]
    with open(FileName+str(i), mode='wb') as file: # b is important -> binary
      file.write(MemoryContent[i])
    includeStr = includeStr +" "+ str(i)
  print (includeStr)
  
  # IMPORTANT: pad array with zeros if you use only 3 segments (see above)
  while len(startArray) < 16 :
    startArray =startArray + struct.pack("I",0)
    endArray =  endArray   + struct.pack("I",0)
  # debug print (binascii.hexlify(startArray))
  # debug print (binascii.hexlify(endArray))
  if (len(endArray) + len (startArray)) != 32 :
    print("ERROR: please make sure you add / remove padding if you change the semgents.")

if  fileContent.find( FILENAMEDUMMY) < 0:
    print("ERROR: FILENAMEDUMMY dummy not found in binary")
else:
    BinaryFileName=os.path.basename(FileName) +"\0"
    if len(BinaryFileName) >64:								# check that filename is <48 chars
        BinaryFileName=BinaryFileName[0:64]					# truncate if necessary. 49th char in ESP is zero already
    else:
        BinaryFileName= BinaryFileName.ljust(64,'\0');		# pad with zeros.
    
    fileContent=fileContent.replace(FILENAMEDUMMY,BinaryFileName)

if firmware_esp8266:
  if  fileContent.find( MD5DUMMY) < 0:
    print("ERROR: MD5 dummy not found in binary")
  else:
    hashString=hashString.replace (MD5DUMMY,"",1)
    m = md5(hashString) #use segment 1
    md5hash = m.digest()
    print("MD5 hash: "+ m.hexdigest())
    fileContent=fileContent.replace(MD5DUMMY,md5hash+startArray+endArray)

print("\nwriting output file:\n" + FileName)
with open(FileName, mode='wb') as file: # b is important -> binary
  file.write(fileContent)

#k=input("press close to exit")
