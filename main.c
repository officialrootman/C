#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX_PORTS 10
#define BUFFER_SIZE 4096
#define MAX_CONNECTIONS 50

// ANSI Color codes for better visualization
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

typedef struct {
    int port;
    char *service_name;
} Port_Service;

Port_Service monitored_ports[] = {
    {21, "FTP"},
    {22, "SSH"},
    {23, "Telnet"},
    {25, "SMTP"},
    {80, "HTTP"},
    {443, "HTTPS"},
    {3306, "MySQL"},
    {5432, "PostgreSQL"},
    {8080, "HTTP-Alt"},
    {27017, "MongoDB"}
};

volatile sig_atomic_t running = 1;

void print_banner() {
    printf(CYAN);
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║          Terminal HoneyPot v1.0          ║\n");
    printf("║       Created by: officialrootman       ║\n");
    printf("║      Real-time Attack Monitoring        ║\n");
    printf("╚═══════════════════════════════════════════╝\n");
    printf(RESET);
}

void handle_signal(int sig) {
    running = 0;
    printf(YELLOW "\n[*] Shutting down honeypot...\n" RESET);
}

void print_timestamp() {
    time_t now;
    time(&now);
    char *date = ctime(&now);
    date[strlen(date) - 1] = '\0';  // Remove newline
    printf("[%s] ", date);
}

void *handle_connection(void *socket_desc) {
    int sock = *(int*)socket_desc;
    int port = monitored_ports[sock % MAX_PORTS].port;
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);
    
    getpeername(sock, (struct sockaddr*)&addr, &addr_size);
    inet_ntop(AF_INET, &addr.sin_addr, client_ip, sizeof(client_ip));
    
    print_timestamp();
    printf(GREEN "[+] New connection from %s to port %d (%s)\n" RESET, 
           client_ip, port, monitored_ports[sock % MAX_PORTS].service_name);

    // Receive data
    ssize_t read_size;
    while ((read_size = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        print_timestamp();
        printf(RED "[!] Received data on port %d (%s) from %s:\n" RESET, 
               port, monitored_ports[sock % MAX_PORTS].service_name, client_ip);
        printf(MAGENTA "%s\n" RESET, buffer);

        // Send honeypot response
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), 
                "Welcome to %s service. This connection is being monitored.\n", 
                monitored_ports[sock % MAX_PORTS].service_name);
        send(sock, response, strlen(response), 0);
    }

    print_timestamp();
    printf(BLUE "[-] Connection closed from %s on port %d\n" RESET, client_ip, port);
    
    close(sock);
    free(socket_desc);
    return NULL;
}

int create_socket(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf(RED "Failed to create socket for port %d\n" RESET, port);
        return -1;
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        printf(RED "setsockopt failed for port %d\n" RESET, port);
        return -1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf(RED "Bind failed for port %d\n" RESET, port);
        return -1;
    }

    listen(sock, MAX_CONNECTIONS);
    return sock;
}

int main() {
    signal(SIGINT, handle_signal);
    print_banner();

    int server_sockets[MAX_PORTS];
    fd_set readfds;
    int max_sd = 0;

    // Initialize server sockets
    for (int i = 0; i < MAX_PORTS; i++) {
        server_sockets[i] = create_socket(monitored_ports[i].port);
        if (server_sockets[i] > max_sd) {
            max_sd = server_sockets[i];
        }
    }

    printf(GREEN "\n[*] Honeypot is running. Monitoring following ports:\n" RESET);
    for (int i = 0; i < MAX_PORTS; i++) {
        if (server_sockets[i] != -1) {
            printf(YELLOW "    → Port %d (%s)\n" RESET, 
                   monitored_ports[i].port, monitored_ports[i].service_name);
        }
    }
    printf(GREEN "\n[*] Press Ctrl+C to stop the honeypot\n\n" RESET);

    while (running) {
        FD_ZERO(&readfds);
        for (int i = 0; i < MAX_PORTS; i++) {
            if (server_sockets[i] != -1) {
                FD_SET(server_sockets[i], &readfds);
            }
        }

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            printf(RED "Select error\n" RESET);
            continue;
        }

        for (int i = 0; i < MAX_PORTS; i++) {
            if (server_sockets[i] != -1 && FD_ISSET(server_sockets[i], &readfds)) {
                struct sockaddr_in client;
                socklen_t client_len = sizeof(client);
                int *new_sock = malloc(sizeof(int));
                *new_sock = accept(server_sockets[i], (struct sockaddr*)&client, &client_len);
                
                if (*new_sock < 0) {
                    free(new_sock);
                    continue;
                }

                pthread_t thread_id;
                if (pthread_create(&thread_id, NULL, handle_connection, (void*)new_sock) < 0) {
                    printf(RED "Could not create thread\n" RESET);
                    free(new_sock);
                    continue;
                }
                pthread_detach(thread_id);
            }
        }
    }

    // Cleanup
    for (int i = 0; i < MAX_PORTS; i++) {
        if (server_sockets[i] != -1) {
            close(server_sockets[i]);
        }
    }

    return 0;
}