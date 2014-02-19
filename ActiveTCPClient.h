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
#include <condition_variable>

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
    STOPED,
};

class ActiveTCPClient {
public:
    // Este é o único construtor público da classe, recebe como parâmetros
    // o IPv4 e porta do servidor TCP. A conexão não é realizada até que o 
    // método Start() seja chamado.
    ActiveTCPClient(const string& remote_ip, const uint16_t remote_port);
    
    //Um usuário desta classe deve chamar este método logo após o construtor.
    //Uma thread é criada e iniciada em background. Essa thread monitora o
    //estado da conexão e reconecta quando há erros de leitura, exceto quando
    //o estado do cliente é SOCKET_STATE::CLOSED. Neste caso assume-se que o 
    //cliente suspendeu propositalmente a conexão.
    bool         Start();
    
    //Fecha o socket e muda o estado do cliente para SOCKET_STATE::CLOSED. 
    //Deve-se chamar este método para fechar a conexão e não tentar
    //automaticamente a reconxão.
    virtual void Close();
    
    //Fecha o socket e encerra a thread de monitoramento. Deve ser chamada
    //apenas quando este cliente não for mais ser utilizado.
    void         Stop();
    
    virtual ~ActiveTCPClient();
        
    //Retorna uma string que representa o estado atual do cliente
    virtual string GetState() const;
    
    //Retorna o último erro gerado pela operação do cliente. Ver possíveis
    //valores na enum SOCKET_ERROR
    virtual SOCKET_ERROR GetError() const { return last_error; }
    
    //Retorna a próxima linha de texto entregue pelo servidor (incluindo o
    //caracter 0xA ou '\n' no final). Este método é bloqueante - ele aguarda
    //tanto pela chegada da mensagem em si quanto pelo estabelecimento da 
    //conexão TCP. Essa espera contorna o problema de desconexões constantes 
    //pelo 'outro lado'.
    virtual string NextLine(size_t maxlen = 255);
    
    //Opcionalmente pode ser implementado este método, que retornaria apenas o
    //próximo caracter ASCII (um byte)
    virtual char   NextChar();
    
    //Opcionalmente pode ser implementado este método, que retornaria um bloco
    //de texto de um tamanho arbitrário, independente de delimitadores.
    virtual string NextChunk(size_t length);
    

private:
    //Este construtor é privado então não se pode instanciar esta classe sem
    //informar IP e porta do servidor, mas note que a implementação desse
    //construtor está usando valores padrão (ip: locallhost e porta 7000). 
    //Talvez seja conveniente tornar este construtor público e construir uma 
    //instância usando esses valores padrão.
    ActiveTCPClient();
    
    //Construtor de cópia é privado mesmo
    ActiveTCPClient(const ActiveTCPClient& orig);
    
    //Método privado que executa a tentativa de conexão com o servidor.
    //Quando a conexão é estabelecida a variável condicional 'socket_is_connected'
    //é notificada (potencialmente destravando uma chamada a NextLine())
    virtual bool Connect();
    virtual bool Connect(const string& ip, const uint16_t port);
    
    //Ponto de entrada da thread de monitoramento
    void run_monitor();
    
    SOCKET_STATE        state;
    int                 sockfd;
    string              remote_address;
    short               remote_port;
    
    //Esse campo guarda as leituras parciais antes da chegada do 0xA
    string              partial_line;
    
    std::thread _monitor_thread;
    
    
    SOCKET_ERROR last_error;
    
    //Essas duas variáveis controlam a dança entre o Connect e o NextLine.
    //Na entrada de NextLine espera-se indefinidamente até que o predicado 
    //atrelado a essa condiçional esteja 'true' (ver implementação). 
    //No loop da thread de controle espera-se indefinidamente que o predicado 
    //seja 'false' (ou seja, que o socket esteja desconectado), então o controle
    //chama Connect(). No final de Connect, socket_is_connected é sinalizado, e
    //quem estiver ouvindo (neste caso a NextLine) é avisado de que o valor do
    //predicado mudou.
    std::condition_variable socket_is_connected;
    std::mutex cnx_mutex;
    
    //Teste usado para implementar o sistema de sinalização de socket conectado/
    //desconectado.
    inline bool is_connected();
    
    error_t __errno;
    
    inline void clear_errors() { __errno = 0; last_error = SOCKET_ERROR::NO_ERROR; }
};

#endif	/* ACTIVETCPCLIENT_H */

