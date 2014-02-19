/* 
 * File:   main.cpp
 * Author: Moyses
 *
 * Created on February 17, 2014, 9:22 PM
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <cstdlib>
#include <iostream>

#include "ActiveTCPClient.h"
using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

static bool running = true;

static void ctrlc_handler(int sig)
{
    cout << "Saindo...\n";
    running = false;
}

/*
 * 
 */
int main(int argc, char** argv)
{
    signal(SIGINT, ctrlc_handler);
    signal(SIGTERM, ctrlc_handler);

    ActiveTCPClient tcpClient("192.168.0.198", 7000);
    bool ret = false;
    ret = tcpClient.Start();

    if (ret == false) {
        cerr << " Nao deu " << endl;
        exit(1);
    }

    while (running) {
        string line = tcpClient.NextLine();
        std::cout << line << std::endl;
        std::cout << "..." << std::endl;
    }

    cout << "Fechando o cliente TCP...";
    tcpClient.Close();
    cout << "ok\n";
    return 0;
}

