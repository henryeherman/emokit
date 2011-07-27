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
#include "libepoc.h"


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
    
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
    if(bind(server_sockfd, (struct sockaddr *)&server_address,server_len)<0){
        error("ERROR on binding");
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
    
	while(1)
	{
        uint64_t count = 0;
        listen(server_sockfd, 5);
        client_len = sizeof(client_address);
        printf("Wainting for client ....\r\n");
        client_sockfd = accept( server_sockfd, (struct sockaddr *)&client_address, &client_len);
        printf("Connect to EPOC\r\n");
        d = epoc_create();
        printf("Current epoc devices connected: %d\n", epoc_get_count(d, EPOC_VID, EPOC_PID));
        if(epoc_open(d, EPOC_VID, EPOC_PID, 0) != 0)
        {
            printf("CANNOT CONNECT\r\n");
            return 1;
        }
        
		while(epoc_read_data(d, data) > 0)
		{
            uint32_t n = 0;
			epoc_get_next_frame(&frame, data);
            /*frame.F3\FC6\P7\T8\F7\F8\T7\P8\AF4\F4\AF3\O2\O1\FC5*/
            sprintf(buffer,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", frame.gyroX, frame.gyroY, frame.F3, 
                                       frame.FC6, frame.P7, frame.P7, frame.T8,
                                       frame.F7, frame.F8, frame.T7, frame.P8,
                                       frame.AF4, frame.F4, frame.AF3, frame.O2,
                                       frame.O1, frame.FC5);
                                       
            //printf("Writing.\r\n");
            n = send(client_sockfd, buffer, strlen(buffer),0);
            //printf("Done.\r\n");
            count = count + n; 
            if(n < 0) {
                printf("Write failed");
                break;
            } else {
                //printf("%lu\r",(unsigned long)count);
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
