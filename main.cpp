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

#include "ActiveTCPClient.h"
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

    ActiveTCPClient tcpClient("192.168.1.16", 7020);
    bool ret = false;
    ret = tcpClient.Connect();

    if (ret == false) {
        cerr << " Nao deu " << endl;
    }

    string line = tcpClient.NextLine();
    while (line.compare("") != 0) {
        std::cout << line << std::endl;
        std::cout << "..." << std::endl;
        sleep(1);
        line = tcpClient.NextLine();
    }

    tcpClient.Close();

    return 0;
}

