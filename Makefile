C = gcc
all: 
	$(CC) xinying_udp_server.c -o server
	$(CC) xinying_udp_client.c -o client
server: 
	  $(CC) xinying_udp_server.c -o server
client:
	  $(CC) xinying_udp_client.c -o client
clean:
	$(RM) server clien
