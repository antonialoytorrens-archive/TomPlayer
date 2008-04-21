import socket
import Image
import sys

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
s.connect(('192.168.10.70', 2007))
s.send("getscreen")
i = 0
data="" 
try :
	while True :
		data += s.recv(1500)		
except :
	print "comm over"	
	
l =  len(data)/2
RGBBuff = ""

#f=open("date.raw","w")
#f.write(data)
#f.close()

for i in xrange(l) :
	val16 = ord(data[i*2:i*2+1])  + (ord(data[i*2+1:i*2+2])<<8)	
	R = chr((val16 & 0xF800) >> 8)
	G = chr((val16 & 0x07E0) >>3)
	B = chr((val16 & 0x1F)<<3)
#	print "0x%X %X %X %X" % (val16,ord(R),ord(G),ord(B))
	RGBBuff += R+G+B
		
im = Image.fromstring("RGB", (480,272),RGBBuff)
im.save(sys.argv[1])
		