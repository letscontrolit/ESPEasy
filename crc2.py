 

import sys
import binascii
import struct
import md5
MD5DUMMY = "MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere..."
MemorySegmentStart,MemorySegmentEnd,MemoryContent=[],[],[]
 
##################################################################
# this subroutine shows the segments of a  part
##################################################################
def showSegments (fileContent,offset):
    global MemorySegmentStart, MemorySegmentEnd, MemoryContent
    header = struct.unpack("ii", fileContent[offset:offset+8])
    herestr =""
    MemorySegmentStart.append(struct.pack("I",header[0]))
    MemorySegmentEnd.append(struct.pack("I",header[0]+header[1])) 
    MemoryContent.append(fileContent[offset+8:offset+8+header[1]])
    if  fileContent.find( MD5DUMMY, offset+8, offset+8+header[1]) >0 :
        herestr= " <-- CRC is here."
    print ("SEGMENT "+ str(len(MemorySegmentStart)-1)+ ": memory position: " + hex(header[0])+" to " + hex(header[0]+header[1]) +  " length: " + hex(header[1])+herestr)
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
FileName = sys.argv[1]#"C:/ArduinoPortable/sketchbooks/build/sketch_jan15a.ino.bin"
#FileName =  "C:/ArduinoPortable/sketchbooks/build/sketch_jan15a.ino.bin"
print( "\n\nReplacing dummy MD5 checksum in .bin file")



with open(FileName, mode='rb') as file: # b is important -> binary
    nextpos =0;
    fileContent = file.read()
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

if  fileContent.find( MD5DUMMY) < 0:
    print("ERROR: MD5 dummy not found in binary")
else:
    hashString=hashString.replace (MD5DUMMY,"",1) 
    m = md5.new()
    m.update (hashString) #use segment 1
    md5hash = m.digest()
    print("MD5 hash: "+ m.hexdigest())
    print("\nwriting output file:\n" + FileName)
    with open(FileName, mode='wb') as file: # b is important -> binary
      file.write(fileContent.replace(MD5DUMMY,md5hash+startArray+endArray))
#k=input("press close to exit") 
