#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <map>
#include <mutex>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 100

std::map<int, int> clients; // fd -> client ID
std::mutex clients_mutex;

void setNonBlocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void broadcastMessage(int sender_fd, const std::string &msg) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto &[fd, id] : clients) {
        if (fd != sender_fd) send(fd, msg.c_str(), msg.size(), 0);
    }
}

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

    setNonBlocking(server_fd);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) { perror("epoll_create1"); return 1; }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);

    epoll_event events[MAX_EVENTS];
    int client_counter = 1;

    std::cout << "Server started on port " << PORT << std::endl;

    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) { perror("epoll_wait"); break; }

        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                // New client
                sockaddr_in client_addr{};
                socklen_t addrlen = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                if (client_fd < 0) { perror("accept"); continue; }

                setNonBlocking(client_fd);

                epoll_event ev{};
                ev.events = EPOLLIN | EPOLLET; // edge-triggered
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

                {
                    std::lock_guard<std::mutex> lock(clients_mutex);
                    clients[client_fd] = client_counter++;
                }
                std::cout << "New client connected: fd=" << client_fd << std::endl;

            } else {
                // Existing client message
                char buffer[BUFFER_SIZE];
                int bytes = recv(fd, buffer, BUFFER_SIZE, 0);

                if (bytes <= 0) {
                    std::lock_guard<std::mutex> lock(clients_mutex);
                    int id = clients[fd];
                    std::cout << "Client " << id << " disconnected" << std::endl;
                    close(fd);
                    clients.erase(fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    continue;
                }

                buffer[bytes] = '\0';
                int sender_id = clients[fd];
                std::string msg = "Client " + std::to_string(sender_id) + ": " + buffer;
                std::cout << msg << std::endl;

                broadcastMessage(fd, msg);
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
