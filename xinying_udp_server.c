#include <sys/types.h>
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
#include <sys/time.h>
#include <memory.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
/* server */

#define MAXBUFSIZE 10000

int main (int argc, char * argv[] )
{

	int sock;				// This will be our socket
	struct sockaddr_in sin, remote;		// Internet socket address structure
	unsigned int remote_length;		// length of the sockaddr_in structure
	int nbytes;				// number of bytes we receive in our message
	int response_length;
	char buffer[MAXBUFSIZE];		// a buffer to store our received message
	char buffer2[MAXBUFSIZE];		// second buffer to store received message
	char response[MAXBUFSIZE];		// server response
	char fname[MAXBUFSIZE];
	char fsize[256];
	FILE *fp;
	char *search_result;
	int size, size_per_send, i;
	int index;
	char cwd[256];
	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}

	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	bzero(&sin,sizeof(sin));                    //zero the struct
	sin.sin_family = AF_INET;                   //address family
	sin.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	sin.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine


	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create socket");
	}


	/******************
	  Once we've created a socket, we must bind that socket to the 
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("unable to bind socket\n");
	}
	
	while(1){
		remote_length = sizeof(remote);
        	//waits for an incoming message
        	bzero(buffer,sizeof(buffer));
        	nbytes  = recvfrom(sock, buffer, MAXBUFSIZE, 0,
                                        (struct sockaddr *)&remote, &remote_length);
        	printf("The client says %s\n", buffer);
		if (strstr(buffer, "put")){ 
			
			strcpy(response, "Receiving...");
			response_length = sizeof(response);
			nbytes = sendto(sock, response, response_length, 0,
				(struct sockaddr *)&remote, remote_length);
			
			recvfrom(sock, buffer2, MAXBUFSIZE, 0,
                                (struct sockaddr *)&remote,&remote_length);
			printf("Client says: %s \n", buffer2);

			char* sec_arg = strstr(buffer, " ");
                        strcpy(fname, sec_arg+1);

			if (strstr(buffer2, "Sending")){
                                fp=fopen(fname,"wb");
                        	char* sec_arg = strstr(buffer2, "\n");
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
				printf("%s \n", response);				
			} else {
				//printf("Client says failed to open file.\n");
			}

		} else if (search_result = strstr(buffer, "get")){ 
			
			char* sec_arg = strstr(buffer, " ");
			strcpy(fname, sec_arg+1);
			fp = fopen(fname, "rb");
			if (fp == NULL){
				printf("Failed open file %s.\n", fname);
				strcpy(response, "Failed open file.");
				response_length = sizeof(response);
				nbytes = sendto(sock, response, response_length, 0,
					 		(struct sockaddr*)&remote, remote_length);
			} else {
				printf("Find file %s. Sending...\n", fname);
				// sending msg and file size
				fseek(fp, 0, SEEK_END); // seek to end of file
				int size = ftell(fp); // get current file pointer
				fseek(fp, 0, SEEK_SET); // seek back to beginning of file
				sprintf(fsize, "%d", size);
                                printf("file size %s \n", fsize);
				// sending msg and file size
				strcpy(response, "Find file. Sending...\n");
				strcat(response, fsize);
				response_length = sizeof(response);
                                nbytes = sendto(sock, response, response_length, 0,
                                                        (struct sockaddr*)&remote, remote_length);
				// check file size and send file
                                int repeats = (int) (size/MAXBUFSIZE) + 1;
                                for (i=0; i<repeats; i++){
                                        size_per_send = (MAXBUFSIZE) < (size-i*MAXBUFSIZE) ? (MAXBUFSIZE):(size-i*MAXBUFSIZE);
                                        int readed = fread(response, sizeof(char), size_per_send, fp);
                                        printf("readed bytes %d \n", readed);
                                        response_length = sizeof(response);
                                        nbytes = sendto(sock, response, response_length, 0,
                                                        (struct sockaddr*)&remote, remote_length);
                                }

				// file received msg
                                recvfrom(sock, buffer2, MAXBUFSIZE, 0,
                                                (struct sockaddr *)&remote, &remote_length);
                                printf("Client says: %s \n", buffer2);
				fclose(fp);
			}
	
		} else if (strcmp(buffer, "ls")==0) {
			struct dirent **namelist;
    			int n;
			// get working directory path
			if (getcwd(cwd, sizeof(cwd)) == NULL) {
      				perror("getcwd() error");
				strcpy(response, "get working directory path error. \n");
			}
    			else {
      				printf("current working directory is: %s\n", cwd);
				strcpy(response, cwd);
				strcpy(response, "\n");
  			}
			// get directory contents
    			n = scandir(".", &namelist, NULL, alphasort);
    			if (n < 0) {
				perror("scandir");
				strcat(response, "Error in getting directory content.\n");
			}
    			else {
        			while (n--) {
            				printf("%s\n", namelist[n]->d_name);
            				strcat(response, namelist[n]->d_name);
					strcat(response, "\n");
					free(namelist[n]);
        			}
        			free(namelist);
    			}
			response_length = sizeof(response);
			nbytes = sendto(sock, response, response_length, 0, 
						(struct sockaddr *)&remote, remote_length);

		
		} else if (strcmp(buffer, "exit")== 0) {
			strcpy(response, "Exit? yes/no \n");
			response_length = sizeof(response);
			nbytes = sendto(sock, response, response_length, 0,
                                                (struct sockaddr *)&remote, remote_length);
			nbytes = recvfrom(sock, buffer2, MAXBUFSIZE, 0,
                                        (struct sockaddr *)&remote, &remote_length);
			printf("Client says:%s \n", buffer2);
			if (strcmp(buffer2, "yes")) {break;}
			if (strcmp(buffer2, "no")) {continue;}

		} else {
			// handle invalid commands
			/*
			strcpy(response, "\"");
			strcat(response, "\" is invalid command!\n");
			strcat(response, "Input put, get, ls or exit.\n");
			response_length = sizeof(response);
			nbytes = sendto(sock, response, response_length, 0,
						(struct sockaddr *)&remote, remote_length);
			*/
		}

	}

	close(sock);
}

