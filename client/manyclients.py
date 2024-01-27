import client
import argparse
from threading import Thread
from datetime import datetime

parser = argparse.ArgumentParser()

# paso un argumento de nombre number_of_clients de tipo int
parser.add_argument('number_of_clients', type=int)

# obtengo numero de clientes por linea de comandos
clients = parser.parse_args().number_of_clients
print('Creando ' + str(clients) + ' clientes')

threads = []

for x in range (0, clients):

	# chequeo que empiecen en paralelo con el print
	print('[Client ' + str(x) + ']: ' + datetime.now().strftime('%H:%M:%S'))
	
	# arranco los threads
	thread = Thread(target=client.main, args=(x,)) # paso x como client id
	threads.append(thread)
	thread.start()	

# espero a que terminen todos
for thread in threads :
	thread.join()

#print(__file__ + 'Finalizo')