#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>  


#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

struct Client {
    int fd;
    int id;
};

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { perror("bind"); return 1; }
    if (listen(server_fd, 10) < 0) { perror("listen"); return 1; }

    std::cout << "Server started on port " << PORT << std::endl;

    std::vector<pollfd> fds;
    std::vector<Client> clients;
    int client_counter = 1;

    pollfd pfd{};
    pfd.fd = server_fd;
    pfd.events = POLLIN;
    fds.push_back(pfd);

    while (true) {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret < 0) { perror("poll"); break; }

        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == server_fd) {
                    sockaddr_in client_addr{};
                    socklen_t addrlen = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                    if (client_fd < 0) { perror("accept"); continue; }

                    pollfd new_pfd{};
                    new_pfd.fd = client_fd;
                    new_pfd.events = POLLIN;
                    fds.push_back(new_pfd);
                    clients.push_back({client_fd, client_counter});
                    std::cout << "New client connected: " << client_counter++ << std::endl;
                } else {
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    int bytes = recv(fds[i].fd, buffer, BUFFER_SIZE, 0);
                    if (bytes <= 0) {
                        // Client disconnected
                        int id = -1;
                        for (auto &c : clients) {
                            if (c.fd == fds[i].fd) { id = c.id; break; }
                        }
                        std::cout << "Client " << id << " disconnected" << std::endl;
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        clients.erase(std::remove_if(clients.begin(), clients.end(),
                                     [&](Client c){ return c.fd == fds[i].fd; }), clients.end());
                        --i;
                        continue;
                    }

                    // Broadcast message
                    int sender_id = -1;
                    for (auto &c : clients) if (c.fd == fds[i].fd) sender_id = c.id;
                    std::string msg = "Client " + std::to_string(sender_id) + ": " + buffer + "\n";
                    for (auto &c : clients) {
                        if (c.fd != fds[i].fd) send(c.fd, msg.c_str(), msg.size(), 0);
                    }
                    std::cout << msg;
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
