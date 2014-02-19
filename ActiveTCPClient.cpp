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

#include <iostream>


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

bool ActiveTCPClient::Connect(const string& ip, const uint16_t port) {
    state = CONFIGURED;
    remote_address.assign(ip);
    remote_port = port;
    return this->Connect();
}

string ActiveTCPClient::GetState() const {

    return state_names[state];
}

bool ActiveTCPClient::Connect() {

    //Será que o remote_address é um endereço IPv4 válido?
    //Aqui eu não me preocupo com isso...
    if (remote_address.empty() || remote_port == 0) {
        last_error = SOCKET_ERROR::PARAMETER_ERROR;
        return false;
    }

    if (state == CONNECTED || sockfd > 0) {
        Close();
    }
    int err = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        //log "ERROR opening socket"
        state = SOCKET_STATE::ERROR;
        return false;
    }

    //Eu prefiro usar getaddrinfo para instanciar um sockaddr*
    //principalmente por que nao preciso me preocupar com DNS.
    struct addrinfo hint_info = {0};
    hint_info.ai_family = AF_INET;
    hint_info.ai_protocol = IPPROTO_TCP;
    hint_info.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    struct addrinfo* srv_info;
    string port_str = std::to_string(remote_port);
    err = getaddrinfo(remote_address.c_str(),
            port_str.c_str(),
            &hint_info, &srv_info);

    if (err || srv_info->ai_addr == NULL) {
        //log "ERROR, no such host;
        state = SOCKET_STATE::ERROR;
        __errno = err;
        last_error = SOCKET_ERROR::NETWORK_ERROR;
        return false;
    }
    //O bloco a seguir configura o reuso do ip e porta para reconexões
    {
        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) {
            state = SOCKET_STATE::ERROR;
            __errno = err;
            last_error = SOCKET_ERROR::NETWORK_ERROR;
            return false;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof (int)) == -1) {
            state = SOCKET_STATE::ERROR;
            __errno = err;
            last_error = SOCKET_ERROR::NETWORK_ERROR;
            return false;
        }
    }

    if (connect(sockfd, srv_info->ai_addr, srv_info->ai_addrlen) < 0) {
        cout << "ERROR connecting: " << strerror(errno) << "\n";
        state = SOCKET_STATE::ERROR;
        last_error = SOCKET_ERROR::NETWORK_ERROR;
        __errno = errno;
        return false;
    }
    state = SOCKET_STATE::CONNECTED;
    last_error = SOCKET_ERROR::NO_ERROR;

    //Se houver uma chamada bloqueada em NextLine() neste momento, ela vai acordar.
    socket_is_connected.notify_all();
    return true;
}

ActiveTCPClient::ActiveTCPClient(const ActiveTCPClient& orig) {
    //empty by design
}

ActiveTCPClient::~ActiveTCPClient() {
    Close();
}

bool ActiveTCPClient::is_connected() {
    return sockfd > 0 && state == SOCKET_STATE::CONNECTED;
}

string ActiveTCPClient::NextLine(size_t maxlen) {

    char tmp_buf[32] = {0};
    while (partial_line.find("\n") == string::npos &&
            partial_line.size() < maxlen) {

        //Este lock garante que o NextLine não prossegue no meio de uma tentativa
        //de (re)conexão pela thread de monitoramento
        std::unique_lock<std::mutex> lock(cnx_mutex);

        //cout << "NextLine() foi chamada mas o socket está desconectado...\n";
        socket_is_connected.wait(lock,
                [this] {
                    //quando alguém chamar socket_is_connected.notify_all ou 
                    //socket_is_connected.notify_one esse predicado é testado;
                    //se for verdadeiro o wait é interrompido. se for falso
                    //continua esperado.
                    return this->is_connected();
                });

        int len = recv(sockfd, tmp_buf, 32, 0);
        if (len <= 0) {
            __errno = errno;
            last_error = SOCKET_ERROR::IO_ERROR;
            Close();
            socket_is_connected.notify_all(); //avisa a thread de controle
            
            continue;   //vai voltar ao socket_is_connected.wait() ali em cima.
                        //(outra opção seria retornar o que já foi recebido em
                        // partial_line e deixar quem chamou NextLine decidir se
                        // quer continuar tentando ou não)
        } else {
            tmp_buf[len] = '\0';
            partial_line.append(tmp_buf);
        }
    }

    string line_str;
    if (partial_line.size() == maxlen) {
        line_str = partial_line.append("\n");
        partial_line.clear();
    } else {
        line_str = partial_line.substr(0, partial_line.find("\n"));
        partial_line.assign(partial_line.substr(partial_line.find("\n") + 1));
    }

    clear_errors();
    return line_str;
}

char ActiveTCPClient::NextChar() {

    return '\0';
}

string ActiveTCPClient::NextChunk(size_t length) {

    return "";
}

void ActiveTCPClient::run_monitor() {
    while (state != SOCKET_STATE::STOPED) {
        std::unique_lock<std::mutex> lock(cnx_mutex);
        socket_is_connected.wait(lock,
                [this] {
                    return this->is_connected() == false;
                });
        Connect();
        sleep(3);
    }
}

void ActiveTCPClient::Close() {
    if (shutdown(sockfd, SHUT_RDWR) == 0) {
        close(sockfd);
    }
    sockfd = -1;
    state = SOCKET_STATE::CLOSED;
}

bool ActiveTCPClient::Start() {
    _monitor_thread = std::thread([this] { this->run_monitor(); });

    _monitor_thread.detach();

    return true;
}

void ActiveTCPClient::Stop() {

    if (state != SOCKET_STATE::CLOSED) {
        Close();
    }
    state = SOCKET_STATE::STOPED;
}