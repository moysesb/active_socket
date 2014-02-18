/* 
 * File:   main.cpp
 * Author: Moyses
 *
 * Created on February 17, 2014, 9:22 PM
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <cstdlib>
#include <iostream>
using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

/*
 * 
 */
int main(int argc, char** argv)
{
    int sockfd, portno, n;

    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    
    struct addrinfo hint_info = {0};
    hint_info.ai_family = AF_INET;
    hint_info.ai_protocol = IPPROTO_TCP;
    hint_info.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
    
    struct addrinfo* srv_info; 
    n = getaddrinfo("192.168.1.16", "7000", &hint_info, &srv_info);
    
    if (srv_info->ai_addr == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    if (connect(sockfd, srv_info->ai_addr, srv_info->ai_addrlen) < 0)
        error("ERROR connecting");
    
    n  = recv(sockfd, buffer, 256, 0);
    
    sleep(2);
    buffer[n] = '\0';
    std::cout << (const char*) buffer << std::endl;
    close(sockfd);
    return 0;
}

