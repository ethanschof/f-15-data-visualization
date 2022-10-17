//
//  socket.cpp
//  ClientTest
//
//  Created by F15 Capstone on 10/9/22.
//

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

class Communications{
public:
    int client_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[8192] = {0};

    void startConnection(char* serverIP, int port){
        client_fd = socket(AF_INET, SOCK_STREAM, 0);
        setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
        
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        if (inet_pton(AF_INET, serverIP, &address.sin_addr) <= 0) {
            printf("\nInvalid address/ Address not supported \n");
            exit(-1);
        }
        
        bind(client_fd, (struct sockaddr *)&address, sizeof(address));
        connect(client_fd, (struct sockaddr *)&address, sizeof(address));
    }
    //closes TCP connection
    void endConnection(){
        close(client_fd);
    }
    /**
            reads upto 8192 bytes per function call
     */
    unsigned char* getPacket(){
        memset(buffer, '\0', sizeof(buffer));
        read(client_fd, buffer, 8192);
        return (unsigned char*)buffer;
    }
};
