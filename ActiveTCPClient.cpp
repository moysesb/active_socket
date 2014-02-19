/* 
 * File:   ActiveSocket.cpp
 * Author: Moyses
 * 
 * Created on February 18, 2014, 6:30 AM
 */

#include "ActiveTCPClient.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


static const char* state_names[] = {
    "UNKNOWN",
    "CONFIGURED",
    "CONNECTED",
    "ERROR",
    "CLOSED",
};

ActiveTCPClient::ActiveTCPClient() : 
        state(UNKNOWN), 
        sockfd(0),
        remote_address("localhost"), 
        remote_port(DEFAULT_SERVER_PORT),
        last_error(NO_ERROR) {
}

ActiveTCPClient::ActiveTCPClient(const string& remote_ip, 
                                        const uint16_t remote_port) :
        state(CONFIGURED), 
        sockfd(0),
        remote_address(remote_ip),
        remote_port(remote_port),
        last_error(NO_ERROR) {
    
}

bool ActiveTCPClient::connect(const string& ip, const uint16_t port) {
    state = CONFIGURED;
    remote_address.assign(ip);
    remote_port = port;
    return this->connect();
}

bool ActiveTCPClient::connect() {
    if(state != CONFIGURED ||
            remote_address.empty() || remote_port == 0) {
        return false;
    }
    
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
    
}
ActiveTCPClient::ActiveTCPClient(const ActiveTCPClient& orig) {
}

ActiveTCPClient::~ActiveTCPClient() {
}

