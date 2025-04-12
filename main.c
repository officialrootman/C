#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Socket oluştur
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket oluşturulamadı");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Portu bağla
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind başarısız");
        exit(EXIT_FAILURE);
    }

    // Dinlemeye başla
    if (listen(server_fd, 3) < 0) {
        perror("Listen başarısız");
        exit(EXIT_FAILURE);
    }

    printf("Honeypot çalışıyor. Port: %d\n", PORT);

    while (1) {
        // Bağlantı kabul et
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept başarısız");
            exit(EXIT_FAILURE);
        }

        // İstemci IP'sini al
        char *client_ip = inet_ntoa(address.sin_addr);
        printf("Yeni bağlantı: %s\n", client_ip);

        // Saldırganı yanıltıcı mesaj gönder
        char *message = "Sunucu hatası! Lütfen daha sonra tekrar deneyin.\n";
        send(new_socket, message, strlen(message), 0);

        close(new_socket);
    }

    return 0;
}