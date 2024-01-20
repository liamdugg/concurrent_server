# -*- coding: utf-8 -*-
import time
import socket

MAX_SIZE  = 2048
MAX_TIMES = 2
port = 8989
server_ip = '127.0.0.1'

def main(id: int):

	# Creo socket de cliente
	client_sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	client_sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)    
	client_sock.connect((server_ip,port))

	times = 0

	while times < MAX_TIMES :

		data = client_sock.recv(MAX_SIZE).decode()
		
		if data == 'ping' and times < MAX_TIMES:
			times += 1
			client_sock.send(b'pong')
			print('[Server][TH ' + str(id) + '] '+ str(times) + ' --> ' + data)

		time.sleep(1)

	client_sock.send(b'END')
	client_sock.close()