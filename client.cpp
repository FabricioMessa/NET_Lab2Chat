/* Client code in C++ for Lab2 Chat */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <thread>
#include <string>

using namespace std;

//Aqui tomamos un numero y lo convertimos a una cadena de ceros a la izquierda hasta alcanzar un tamaño específico
//Usamos esto porque el protocolo necesita campos de tamaño fijo para saber cuantos bytes debe leer.
string zeroPad(int number, int size) {
    string str = to_string(number); 
    if (str.length() >= size) {
        return str;
    }
    return string(size - str.length(), '0') + str;
}

//Esta funcion lee exactamente N bytes del socket.
string read_exact(int fd, int n) {
    string result = "";
    char buffer[1];
    for (int i = 0; i < n; i++) {
        int bytes_read = read(fd, buffer, 1);
        
        if (bytes_read <= 0) 
            return "";
        
        result += buffer[0];
    }
    return result;
}

//Se crea un hilo separado que se encarga de leer continuamente los mensajes del servidor
void ThreadReadServer(int S) {
    char buff[1000];
    int n, tamano;
    string origin, msg;

    //Lee y procesa los mensajes
    for (;;) {
        n = read(S, buff, 1); //lee el primer byte para saber qué tipo de mensaje llega
        if (n <= 0) {
            cout << "\n[Sistema] Conexion lost." << endl;
            exit(0);
        }

        if (buff[0] == 'm') {
            string size_str = read_exact(S, 7);
            
            if (size_str == "") 
                break;
            
            tamano = stoi(size_str);
            origin = read_exact(S, tamano);
            size_str = read_exact(S, 11);

            if (size_str == "") 
                break;
            
            tamano = stoi(size_str);
            msg = read_exact(S, tamano);

            cout << "\n[" << origin << " (Private)]: " << msg << endl;
            cout << "Select option M, B, Q: ";
            cout.flush();
        } else if (buff[0] == 'b') {
            string size_str = read_exact(S, 7);
            
            if (size_str == "") 
                break;
            
            tamano = stoi(size_str);
            origin = read_exact(S, tamano);
            size_str = read_exact(S, 11);
            
            if (size_str == "") 
                break;

            tamano = stoi(size_str);
            msg = read_exact(S, tamano);

            cout << "\n[" << origin << " (Global)]: " << msg << endl;
            cout << "Select option M, B, Q: ";
            cout.flush();
        }
    }
}

void sendNickname(int S, const string& nickname) {
    string packet = "N";
    packet += zeroPad(nickname.size(), 7);
    packet += nickname;
    write(S, packet.c_str(), packet.size());
}

void sendMessage(int S, const string& destination, const string& msg) {
    string packet = "M";
    packet += zeroPad(destination.size(), 7);
    packet += destination;
    packet += zeroPad(msg.size(), 11);
    packet += msg;
    write(S, packet.c_str(), packet.size());
}

void sendBroadcast(int S, const string& msg) {
    string packet = "B";
    packet += zeroPad(msg.size(), 11);
    packet += msg;
    write(S, packet.c_str(), packet.size());
}

void sendQuit(int S) {
    write(S, "Q", 1);
}

int main(int argc, char* argv[]) {
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (-1 == SocketFD) {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    if (argc < 3) {
        exit(EXIT_FAILURE);
    }

    string server_ip = argv[1];
    int port = atoi(argv[2]);

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    int Res = inet_pton(AF_INET, server_ip.c_str(), &stSockAddr.sin_addr);

    if (0 >= Res) {
        perror("error: invalid IP address");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(SocketFD, (const struct sockaddr*)&stSockAddr, sizeof(struct sockaddr_in))) {
        perror("connect failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    cout << "Conectado al servidor " << server_ip << ":" << port << endl;

    thread(ThreadReadServer, SocketFD).detach();
    cout << "Enter your name: ";
    string nickname;
    getline(cin, nickname);
    sendNickname(SocketFD, nickname);
    
    string selection;
    while (true) {
        cout << "Select option M, B, Q: ";
        getline(cin, selection);

        if (selection == "Q" || selection == "q") {
            sendQuit(SocketFD);
            cout << "[Disconnected]" << endl;
            break;
        } else if (selection == "M" || selection == "m") {
            cout << "Enter destination: ";
            string destination;
            getline(cin, destination);

            cout << "Enter message: ";
            string msg;
            getline(cin, msg);

            sendMessage(SocketFD, destination, msg);
        } else if (selection == "B" || selection == "b") {
            cout << "Enter message: ";
            string msg;
            getline(cin, msg);

            sendBroadcast(SocketFD, msg);
        } else {
            cout << "Opcion invalida. Use M, B o Q." << endl;
        }
    }

    close(SocketFD);
    return 0;
}
