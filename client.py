import socket
import os
import struct

#HOST = ''  # Localhost
HOST = '0.0.0.0'  
PORT = 8080
FULL_PATH_TO_FILE = "/root/project/FreeSSHD.exe"

print "\nName of file to upload:", FULL_PATH_TO_FILE

filesize_in_bytes = os.path.getsize(FULL_PATH_TO_FILE)
print "Size of file to upload:", filesize_in_bytes, "bytes"
print" \nServer will decide executable name and path once uploaded"

print "\nListening for Connection on", HOST, "port", PORT

network_filesize_in_bytes = struct.pack('!I', filesize_in_bytes) # I = 4 byte unsigned int
                                                            # ! = network/big endian


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
conn, addr = s.accept()
print 'Connected by', addr

while 1:
    data = conn.recv(1024)
    if not data: break
    print data
    conn.send("password")
    data = conn.recv(1024)
    if not data: break
    print data
    conn.send(network_filesize_in_bytes)
    f = open(FULL_PATH_TO_FILE, 'rb')
    tot_bytes_sent = 0
    bytes_sent = 0
    while (tot_bytes_sent < filesize_in_bytes):
            l = f.read()
	    if (l):
                print "bytes_read from file: %i" ,len(l)
            bytes_sent = conn.send(l)
            tot_bytes_sent += bytes_sent
            #print('Sent ',repr(l))
            print "bytes_sent: %i", bytes_sent

    f.close()
    print('Done Sending')

    conn.close()



