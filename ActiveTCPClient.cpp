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
last_error(NO_ERROR)
{
}

ActiveTCPClient::ActiveTCPClient(const string& remote_ip,
        const uint16_t remote_port) :
state(CONFIGURED),
sockfd(0),
remote_address(remote_ip),
remote_port(remote_port),
last_error(NO_ERROR)
{

}

bool ActiveTCPClient::Connect(const string& ip, const uint16_t port)
{
    state = CONFIGURED;
    remote_address.assign(ip);
    remote_port = port;
    return this->Connect();
}

bool ActiveTCPClient::Connect()
{
    if (state != CONFIGURED ||
            remote_address.empty() || remote_port == 0) {
        last_error = SOCKET_ERROR::PARAMETER_ERROR;
        return false;
    }

    if (state == CONNECTED) {
        last_error = SOCKET_ERROR::BUSY;
        return false;
    }
    int err = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        //log "ERROR opening socket"
        state = SOCKET_STATE::ERROR;
        return false;
    }

    struct addrinfo hint_info = {0};
    hint_info.ai_family = AF_INET;
    hint_info.ai_protocol = IPPROTO_TCP;
    hint_info.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    struct addrinfo* srv_info;
    string port_str = std::to_string(remote_port);
    err = getaddrinfo(remote_address.c_str(),
            port_str.c_str(),
            &hint_info, &srv_info);

    if (srv_info->ai_addr == NULL) {
        //log "ERROR, no such host;
        state = SOCKET_STATE::ERROR;
        last_error = SOCKET_ERROR::NETWORK_ERROR;
        return false;
    }
    if (connect(sockfd, srv_info->ai_addr, srv_info->ai_addrlen) < 0) {
        //log "ERROR connecting";
        state = SOCKET_STATE::ERROR;
        last_error = SOCKET_ERROR::NETWORK_ERROR;
        __errno = errno;
    }
    state = SOCKET_STATE::CONNECTED;
    last_error = SOCKET_ERROR::NO_ERROR;
    return true;
}

ActiveTCPClient::ActiveTCPClient(const ActiveTCPClient& orig)
{
    //empty by design
}

ActiveTCPClient::~ActiveTCPClient()
{
    if (sockfd > 0) {
        close(sockfd);
    }
}

string ActiveTCPClient::NextLine(size_t maxlen)
{
    if (state != SOCKET_STATE::CONNECTED || sockfd <= 0) {
        last_error = IO_ERROR;
        return "";
    }
    char tmp_buf[32] = {0};
    while (partial_line.find("\n") == string::npos && 
            partial_line.size() <= maxlen) {
        int len = recv(sockfd, tmp_buf, 32, 0);
        if (len <= 0) {
            __errno = errno;
            last_error = SOCKET_ERROR::IO_ERROR;            
            return "";
        } else {
            tmp_buf[len] = '\0';
            partial_line.append(tmp_buf);
        }
    }
    
    string tmp;
    if(partial_line.size() == maxlen) {
        tmp = partial_line.append("\n");
        partial_line.clear();        
    } else {
        tmp = partial_line.substr(0, partial_line.find("\n"));
        partial_line.assign(partial_line.substr(partial_line.find("\n") + 1));
    }
    
    clear_errors();
    return tmp;
}

char ActiveTCPClient::NextChar()
{

    return '\0';
}

string ActiveTCPClient::NextChunk(size_t length)
{

    return "";
}

void ActiveTCPClient::Close()
{

}

void ActiveTCPClient::Start()
{

}

void ActiveTCPClient::Reset() 
{
    
}

void ActiveTCPClient::Stop()
{

}