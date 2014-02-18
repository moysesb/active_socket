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

ActiveTCPClient::ActiveTCPClient() {
    state = UNKNOWN;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

ActiveTCPClient::ActiveTCPClient(const string& remote_ip, const uint16_t remote_port) {
    
}

ActiveTCPClient::ActiveTCPClient(const ActiveTCPClient& orig) {
}

ActiveTCPClient::~ActiveTCPClient() {
}

