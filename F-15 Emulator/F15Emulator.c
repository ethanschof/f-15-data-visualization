/**
 * @file main.c
 * @author C1C Colin Seymour (c23colin.seymour@afacademy.af.edu)
 * @brief takes in the name of the .f15pkt file and reads it in then sends UDP packets to a client on port 2023
 * @version 0.2
 * @date 2022-09-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") //Winsock Library

#define PORT 2023
#define BUFFER_LEN 8192

#define KEYLENGTH 6

char* generatePacket(unsigned char* data, long fSize, int *pkt_len){
    *pkt_len = 0;
    
    //learn packet length (find the keyword)
    while(1){
        if(data[*pkt_len] == '#' && data[*pkt_len+1] == 'm' && data[*pkt_len+2] == 'e' && data[*pkt_len+3] == 'J' && data[*pkt_len+4] == 'o' && data[*pkt_len+5] == 'n'){
            break;
        }else{
            *pkt_len = *pkt_len + 1;
        }
    }

    unsigned char *newPkt = (unsigned char *)malloc(*pkt_len * sizeof(unsigned char));
    for(unsigned long i = 0; i < *pkt_len; i++){
        newPkt[i] = data[i];
    }
    
    //remove used bytes
    long end = fSize - *pkt_len + KEYLENGTH;
    for(unsigned long i = 0; i < end; i++){
        data[i] = data[i+*pkt_len];
    }
    fSize = fSize - *pkt_len;
    return newPkt;
}
/**
 * @brief reads in the name of the .f15pkt file to read in and send to a client
 */
int main( int argc, char *argv[]){
    if(argc == 1){
        printf("Please provide the name of the pkt file.\n");
        system("pause");
        exit(1);
    }else if(argc > 2){
        printf("Too many arguments.\n");
        system("pause");
        exit(1);
    }
    //open file and get info
    FILE *fp;
    fp = fopen(argv[1], "rb");
    if(fp == NULL){
        printf("Failed to open file.\n");
        system("pause");
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long fSize = ftell(fp); 
    printf("File size: %ld bytes.\n", fSize);
    fseek(fp, 0, SEEK_SET);

    //transfer binary into a buffer
    unsigned char *dataBuffer = (unsigned char*)malloc(fSize * sizeof(unsigned char));
    int numPackets = 0;
    for(int i = 0; i < fSize; i++){
        dataBuffer[i] = (unsigned char)fgetc(fp);
        if((i > 4) && (dataBuffer[i-5] == '#' && dataBuffer[i-4] == 'm' && dataBuffer[i-3] == 'e' && dataBuffer[i-2] == 'J' && dataBuffer[i-1] == 'o' && dataBuffer[i] == 'n')){
            numPackets++;
        }
    }
    //create socket connection
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    char c = 0;

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed. Error Code : %d.\n", WSAGetLastError());
        system("pause");
        return 1;
    }

    printf("Initialised Winsock.\n");

    //Create a UDP socket
    if((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d.\n", WSAGetLastError());
        WSACleanup();
        system("pause");
        return 1;
    }

    printf("Socket created. Connecting...\n");
    memset(&server, 0, sizeof server);
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    //Connect to remote server
    if (bind(s, (struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR) {
        printf("Bind error:%d.\nPress a key to exit...", WSAGetLastError());
        WSACleanup();
        system("pause");
        return 1;
    }
    puts("Bind done.\n");
    printf("IP address: %d\n", server.sin_addr.s_addr);

    char* buf;
    struct sockaddr_in si_other;
	int slen , buf_len;
    slen = sizeof(si_other);

    //keep listening for data
	while(1)
	{
		printf("Waiting for data...");
		fflush(stdout);
		
		//clear the buffer by filling null, it might have previously received data
		memset(buf,'\0', BUFFER_LEN);
		
		//try to receive some data, this is a blocking call
		if ((buf_len = recvfrom(s, buf, BUFFER_LEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d" , WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		
		//print details of the client/peer and the data received -- nothing
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Data: %s\n" , buf);
		
		//now reply the client with the packets
        int sentPackets = 0;
        while(sentPackets != numPackets){
            //get next packet to send
            buf = generatePacket(dataBuffer, fSize, &buf_len);

            if (sendto(s, buf, buf_len, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
            {
                printf("sendto() failed with error code : %d" , WSAGetLastError());
                exit(EXIT_FAILURE);
            }
            numPackets++;
        }
	}

    printf("Data successfully transferred. Closing socket.\n\n");
    closesocket(s);
    WSACleanup();
    system("pause");
    return 0;
}