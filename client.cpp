#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>

#define PORT 8080
#define BUFFER_SIZE 1024

// Thread to receive messages from server
void receive_messages(int sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cout << "Server disconnected." << std::endl;
            close(sock);
            exit(0);
        }
        std::cout << buffer << std::endl;
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cout << "Invalid address / Address not supported\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection Failed\n";
        return -1;
    }

    std::cout << "Connected to server!\n";

    std::thread t(receive_messages, sock);
    t.detach();

    std::string message;
    while (true) {
        std::getline(std::cin, message);
        send(sock, message.c_str(), message.length(), 0);
    }

    close(sock);
    return 0;
}
