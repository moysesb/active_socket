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

#define DEFAULT_SERVER_PORT 7000

enum SOCKET_ERROR {
    NO_ERROR,
    NOT_CONFIGURED,
    BUSY,
    IO_ERROR,
    NETWORK_ERROR,
    CONNECTION_REFUSED,
    PARAMETER_ERROR,
    MEMORY_ERROR,
};
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
    virtual ~ActiveTCPClient();

    virtual bool Connect();
    virtual bool Connect(const string& ip, const uint16_t port);
    virtual void Reset();
    
    
    virtual string NextLine(size_t maxlen = 255);
    virtual char   NextChar();
    virtual string NextChunk(size_t length);
    virtual void   Close();
    
    void Start();
    void Stop();
private:
    ActiveTCPClient(const ActiveTCPClient& orig);
    
    SOCKET_STATE        state;
    int                 sockfd;
    string              remote_address;
    short               remote_port;
    
    string  partial_line;
    
    SOCKET_ERROR last_error;
    error_t __errno;
    
    inline void clear_errors() { __errno = 0; last_error = SOCKET_ERROR::NO_ERROR; }
};

#endif	/* ACTIVETCPCLIENT_H */

