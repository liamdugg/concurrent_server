# -*- coding: utf-8 -*-
import time
import socket

MAX_SIZE  = 4096
MAX_TIMES = 2
port = 8989
server_ip = '192.168.0.115'

def main(id: int):

	# Creo socket de cliente
	client_sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	client_sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)    
	client_sock.connect((server_ip,port))

	send_str = b'GET /notfound.html HTTP/1.1\r\nContent-type:text/html\r\n\r\n'

	client_sock.send(send_str)
	data = client_sock.recv(MAX_SIZE).decode()
	
	print('[Cliente] ' + str(id) + ' --> '  + 'Recibido')

	time.sleep(1)
	client_sock.close()
