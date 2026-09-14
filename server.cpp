/* Server code in C++ */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include <iostream>
#include <map>
#include <string>
#include <thread>

using namespace std;

map<string, int> ListOfCli;

string zeroPad(int number, int size) {
    string str = to_string(number);

    if (str.length() >= size)
        return str;

    return string(size - str.length(), '0') + str;
}

void ThreadReadClient(int S) {
    string nickname;
    char buff[1000];
    int n, tamano;

    read(S, buff, 1);
    
    while (buff[0] != 'N') {
        read(S, buff, 1);
    }

    n = read(S, buff, 7);
    buff[n] = '\0';
    tamano = atoi(buff);

    n = read(S, buff, tamano);
    buff[n] = '\0';
    nickname = buff;
    ListOfCli[buff] = S;

    string destination;
    string dataStructure;
    string msg;

    for (;;) {
        n = read(S, buff, 1);

        if (buff[0] == 'M') {
            n = read(S, buff, 7);
            buff[n] = '\0';
            tamano = atoi(buff);

            n = read(S, buff, tamano);
            buff[n] = '\0';
            destination = buff;

            n = read(S, buff, 11);
            buff[n] = '\0';
            tamano = atoi(buff);

            n = read(S, buff, tamano);
            buff[n] = '\0';
            msg = buff;

            dataStructure = dataStructure + 'm';
            dataStructure = dataStructure + zeroPad(nickname.size(), 7);
            dataStructure = dataStructure + nickname;
            dataStructure = dataStructure + zeroPad(msg.size(), 11);
            dataStructure = dataStructure + msg;

            write(ListOfCli[destination], dataStructure.c_str(), dataStructure.size());

        } else if (buff[0] == 'B') {

            n = read(S, buff, 11);
            buff[n] = '\0';
            tamano = atoi(buff);

            n = read(S, buff, tamano);
            buff[n] = '\0';
            msg = buff;

            dataStructure = dataStructure + 'b';
            dataStructure = dataStructure + zeroPad(nickname.size(), 7);
            dataStructure = dataStructure + nickname;
            dataStructure = dataStructure + zeroPad(msg.size(), 11);
            dataStructure = dataStructure + msg;

            for (map<string, int>::iterator it = ListOfCli.begin(); it != ListOfCli.end(); ++it) {
                write(it->second, dataStructure.c_str(), dataStructure.size());
            }

        } else if (buff[0] == 'Q') {
            ListOfCli.erase(nickname);
        }
    }
}

int main(void) {
    struct sockaddr_in stSockAddr;
    int ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buffer[256];
    int n;

    if (-1 == ServerSocket) {
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (-1 == ::bind(ServerSocket, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
        perror("error bind failed");
        close(ServerSocket);
        exit(EXIT_FAILURE);
    }

    if (-1 == listen(ServerSocket, 10)) {
        perror("error listen failed");
        close(ServerSocket);
        exit(EXIT_FAILURE);
    }

    for (;;) {
        int ClientSocket = accept(ServerSocket, NULL, NULL);
        if (0 > ClientSocket) {
            perror("error accept failed");
            close(ServerSocket);
            exit(EXIT_FAILURE);
        }
        thread(ThreadReadClient, ClientSocket).detach();
    }

    close(ServerSocket);
    return 0;
}