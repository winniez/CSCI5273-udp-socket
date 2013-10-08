#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <math.h>
#define MAXBUFSIZE 10000

/* client */

int main (int argc, char * argv[])
{

	int nbytes;                             // number of bytes send by sendto()
	int sock;                               //this will be our socket
	char buffer[MAXBUFSIZE];
	char buffer2[MAXBUFSIZE];
	char response[MAXBUFSIZE];
	char fname[MAXBUFSIZE];
	char fsize[256];
	FILE * fp;
	struct sockaddr_in remote;              //"Internet socket address structure"
	unsigned int remote_length;             // length of the sockaddr_in structure
	int response_length, buffer_length, buffer2_length, size_per_send, i, size;
	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet 
	  i.e the Server.
	 ******************/
	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address

	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create socket");
	}

	while (1){

		/******************
	  	sendto() sends immediately.  
	  	it will report an error if the message fails to leave the computer
	  	however, with UDP, there is no error if the message is lost in the network once it leaves the computer.
	 	******************/
		
		printf("Input command: \n 1. put file \n 2. get file \n 3. ls \n 4. exit \n");
		gets(buffer);
		response_length = sizeof(response);	
		buffer_length = sizeof(buffer);	
		remote_length = sizeof(remote);
		if (strstr(buffer, "put")){
			// send put command
			nbytes = sendto(sock, buffer, buffer_length, 0,
					(struct sockaddr *)&remote, remote_length);
			recvfrom(sock, response, MAXBUFSIZE, 0,
                                        (struct sockaddr *)&remote, &remote_length);
			printf("Server says: %s \n", response);
			char* sec_arg = strstr(buffer, " ");
                        strcpy(fname, sec_arg+1);
                        fp = fopen(fname, "rb");
                        if (fp == NULL){
                                printf("Failed open file %s.\n", fname);
                                strcpy(buffer2, "Failed open file.");
                                buffer2_length = sizeof(buffer2);
                                nbytes = sendto(sock, buffer2, buffer2_length, 0,
                                                        (struct sockaddr*)&remote, remote_length);
                        } else {
				printf("Find file %s. Sending...\n", fname);
                                // sending msg and file size
                                strcpy(buffer2, "Find file. Sending...\n");
				// determine file size
				fseek(fp, 0, SEEK_END); // seek to end of file
                                size = ftell(fp); // get current file pointer
                                fseek(fp, 0, SEEK_SET); // seek back to beginning of file
                                sprintf(fsize, "%d", size);
                                printf("file size %s \n", fsize);
                                strcat(buffer2, fsize);
				buffer2_length = sizeof(buffer2);
                                nbytes = sendto(sock, buffer2, buffer2_length, 0,
                                                        (struct sockaddr*)&remote, remote_length);
				// check file size and send file
				int repeats = (int) (size/MAXBUFSIZE) + 1;
				for (i=0; i<repeats; i++){
					size_per_send = (MAXBUFSIZE) < (size-i*MAXBUFSIZE) ? (MAXBUFSIZE):(size-i*MAXBUFSIZE);
					int readed = fread(buffer2, sizeof(char), size_per_send, fp);
                                	printf("readed bytes %d \n", readed);
                                	buffer2_length = sizeof(buffer2);
                                	nbytes = sendto(sock, buffer2, buffer2_length, 0,
                                                        (struct sockaddr*)&remote, remote_length);
				}	
				// file received msg
				recvfrom(sock, response, MAXBUFSIZE, 0,
						(struct sockaddr *)&remote, &remote_length);
				printf("Server says: %s \n", response);
				fclose(fp);
			}


		} else if (strstr(buffer, "get")){
			// send get command
                        nbytes = sendto(sock, buffer, buffer_length, 0,
                                        (struct sockaddr *)&remote, remote_length);
                        recvfrom(sock, response, MAXBUFSIZE, 0,
                                        (struct sockaddr *)&remote, &remote_length);
                        printf("Server says: %s \n", response);
			// parse file name
			char* sec_arg = strstr(buffer, " ");
                        strcpy(fname, sec_arg+1);
			
			if (strstr(response, "Sending")){
                                fp=fopen(fname,"wb");
				char* sec_arg = strstr(response, "\n");
                                strcpy(fsize, sec_arg+1);
                                size = atoi(fsize);
                                // check file size and receive file
                                int repeats = (int) (size/MAXBUFSIZE)+1;
                                for (i=0; i<repeats; i++){
                                        size_per_send = (MAXBUFSIZE) < (size-i*MAXBUFSIZE) ? (MAXBUFSIZE):(size-i*MAXBUFSIZE);
                                        recvfrom(sock, buffer2, size_per_send, 0,
                                                        (struct sockaddr *)&remote,&remote_length);
                                        fwrite(buffer2, sizeof(char), size_per_send, fp);
                                }

                                fclose(fp);
                                strcpy(response, "File received. \n");
                                response_length = sizeof(response);
                                nbytes = sendto(sock, response, response_length, 0,
                                                (struct sockaddr*)&remote, remote_length); 	
			}


		} else if (strcmp(buffer, "ls") == 0){
			// send ls command
			nbytes = sendto(sock, buffer, buffer_length, 0,
					(struct sockaddr *)&remote, remote_length);
                        recvfrom(sock, response, MAXBUFSIZE, 0,
					(struct sockaddr *)&remote, &remote_length);
                        printf("\n%s\n",response);
		} else if (strcmp(buffer, "exit") == 0){
			nbytes = sendto(sock, buffer, buffer_length, 0,
                                        (struct sockaddr *)&remote, remote_length);
			recvfrom(sock, response, MAXBUFSIZE, 0,
                                        (struct sockaddr *)&remote, &remote_length);
			printf("Server says:%s\n",response);
			// get input
			gets(buffer2);
			// send input to server
			nbytes = sendto(sock, buffer, buffer_length, 0,
                                        (struct sockaddr *)&remote, remote_length);
			if (strcmp(buffer2, "yes\n")) {break;}
                        if (strcmp(buffer2, "no\n")) {continue;}
		} else {
			printf("Invalid command %s. Input command: \n 1. put file \n 2. get file \n 3. ls \n 4. exit \n", buffer);			

		}
	}
	close(sock);

}

