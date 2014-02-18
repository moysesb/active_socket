/* 
 * File:   ActiveSocket.h
 * Author: Moyses
 *
 * Created on February 18, 2014, 6:30 AM
 */

#ifndef ACTIVETCPCLIENT_H
#define	ACTIVETCPCLIENT_H

#include <stdint.h>
#include <sys/types.h>

#include <string>
#include <thread>

using namespace std;

enum SOCKET_STATE {
    UNKNOWN,
    CONFIGURED,
    CONNECTED,
    ERROR,
    CLOSED,
};

class ActiveTCPClient {
public:
    ActiveTCPClient();
    ActiveTCPClient(const string& remote_ip, const uint16_t remote_port);
    ActiveTCPClient(const ActiveTCPClient& orig);
    virtual ~ActiveTCPClient();

    virtual bool connect();
    virtual bool connect(const string& ip, const uint16_t port);
    
    virtual string      next_line(size_t maxlen = 255);
    virtual char        next_char();
    virtual string      next_chunk(size_t length);
    virtual void        close();
    
    void start();
    void stop();
private:
    SOCKET_STATE state;
    int sockfd;
    string remote_address;
    short  remote_port;
    
    string partial_line;
};

#endif	/* ACTIVETCPCLIENT_H */

