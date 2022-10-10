//
//  main.cp
//  ClientTest
//
//  Created by F15 Capstone on 10/9/22.
//  created on the Macbook
//

#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>

#define PORT 37716

class Communications{
public:
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

Communications(){
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);
    std::cout << "[Server] Listening in port " << PORT << "\n";

    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    std::cout << "[Server] Client has successfully connected." << "\n";
    }
};

int main(int argc, const char * argv[]) {
    
    std::cout << "Hello, World!\n";
    return 0;
}
