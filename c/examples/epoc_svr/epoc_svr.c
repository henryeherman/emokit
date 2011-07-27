/* Emotic EPOC daemon that decrypt stream using ECB and RIJNDAEL-128 cipher
 * (well, not yet a daemon...)
 * 
 * Usage: epocd (consumer/research) /dev/emotiv/encrypted output_file
 * 
 * Make sure to pick the right type of device, as this determins the key
 * */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include "libepoc.h"
#include <errno.h>


int main(int argc, char **argv)
{
	FILE *input;
	FILE *output;
	enum headset_type type;
  
	char raw_frame[32];
	struct epoc_frame frame;
	epoc_device* d;
	char data[32];
    char buffer[64];
    char client_resp;
    int err;
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    uint16_t on;
    unsigned long count = 0;
    time_t timestamp; 

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    on=1;
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
    if(bind(server_sockfd, (struct sockaddr *)&server_address,server_len)<0){
        perror("bind() failed");
    }
    
	if (argc < 2)
	{
		fputs("Missing argument\nExpected: epocd [consumer|research|special]\n", stderr);
        return 1;
	}
  
	if(strcmp(argv[1], "research") == 0)
		type = RESEARCH_HEADSET;
	else if(strcmp(argv[1], "consumer") == 0)
		type = CONSUMER_HEADSET;
	else if(strcmp(argv[1], "special") == 0)
		type = SPECIAL_HEADSET;
	else {
		fputs("Bad headset type argument\nExpected: epocd [consumer|research|special] source [dest]\n", stderr);
		return 1;
	}
    printf("Server listening\r\n");
	epoc_init(type);
    
    err=listen(server_sockfd, 5);
	while(1)
	{
        if(err) {
            perror("listen() failed");
        }
        client_len = sizeof(client_address);
        printf("Wainting for client ....\r\n");
        client_sockfd = accept( server_sockfd, (struct sockaddr *)&client_address, &client_len);
        if(client_sockfd < 0) {
            perror("accept() failed");
        }
        printf("Connect to EPOC\r\n");
        d = epoc_create();
        printf("Current epoc devices connected: %d\n", epoc_get_count(d, EPOC_VID, EPOC_PID));
        if(epoc_open(d, EPOC_VID, EPOC_PID, 0) != 0)
        {
            printf("CANNOT CONNECT\r\n");
            return 1;
        }
        count = 0; 
		while(epoc_read_data(d, data) > 0)
		{
            int32_t n = 0;
			epoc_get_next_frame(&frame, data);
            /*frame.F3\FC6\P7\T8\F7\F8\T7\P8\AF4\F4\AF3\O2\O1\FC5*/
            sprintf(buffer,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", 
                                       frame.gyroX, frame.gyroY, frame.F3, 
                                       frame.FC6, frame.P7, frame.P7, frame.T8,
                                       frame.F7, frame.F8, frame.T7, frame.P8,
                                       frame.AF4, frame.F4, frame.AF3, frame.O2,
                                       frame.O1, frame.FC5);
                                       
            //printf("Writing.\r\n");
            n = send(client_sockfd, buffer, strlen(buffer),MSG_NOSIGNAL);
            //printf("Done.\r\n");
            if (n>=0) {
                timestamp = time(NULL);
                count+=1; 
            }
            if(n <= 0) {
                perror("send() failed");
                printf("Write failed\r\n");
                break;
            } else {
                printf("Count,TS:%d,%i\r\n",count, (int)timestamp);

            }
        }
        epoc_close(d);
        printf("Close epoc\r\n");
    }
    close(client_sockfd);
    printf("Close socket\r\n");
    epoc_delete(d);
    printf("Delete epoc\r\n");
	return 0;
}
