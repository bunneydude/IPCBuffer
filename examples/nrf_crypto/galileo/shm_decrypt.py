from Crypto.Cipher import AES
import IPCBuffer
import os

keys = {'0x3b': 1, '0x3d':2, '0x3e':3, '0x37':4, '0x2f':5, '0x1f':6, '0x3f':0}

key = "000102030405060708090a0b0c0d0e0f".decode('hex')
obj = AES.new(key,AES.MODE_ECB)

last_key = 0
current_key = 0

cryptBuffer = IPCBuffer.IPCBuffer(1)
cryptBuffer.open_sketch()

while(1):
	val = cryptBuffer.read()  
	msg = obj.decrypt(val) 
	
#	print(map(hex,map(ord,msg))) #print decrypted data

	current_key = keys[hex(ord(msg[0]) & 0x3f)]
	os.system('clear')
	if(current_key == 0):
		print("Key " + str(last_key) + " released.\n")
	else:
		print("Key " + str(current_key) + " pressed.\n")
	last_key = current_key
