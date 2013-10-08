UPDATE: Transferring large files (e.g. foo1) was not enabled in previous version. This updated version enables transferring large files.


Client file: xinying_udp_client.c
Server file: xinying_udp_server.c
Make file: Makefile

1. input command "make all" to build.
2. Move server and client executable file to separate foler server/ and client/.
3. Move file foo1, foo2, foo3 to previous folders.
4. Execute server. "./server <portnum>
5. Execute client. "./client <host> <portnum>
6. Input commands in client: 
	1) put file 
	2) get file 
	3) ls 
	4) exit 

 
