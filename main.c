#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

void scan_port(const char *ip, int port) {
    int sock;
    struct sockaddr_in target;

    // Soket oluştur
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket oluşturulamadı");
        return;
    }

    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    inet_pton(AF_INET, ip, &target.sin_addr);

    // Bağlantıyı kontrol et
    if (connect(sock, (struct sockaddr *)&target, sizeof(target)) == 0) {
        printf("Port %d açık!\n", port);
    } else {
        printf("Port %d kapalı.\n", port);
    }

    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Kullanım: %s <IP> <BaşlangıçPortu-BitişPortu>\n", argv[0]);
        return 1;
    }

    char *ip = argv[1];
    int start_port, end_port;

    // Port aralığını çözümle
    if (sscanf(argv[2], "%d-%d", &start_port, &end_port) != 2) {
        printf("Port aralığını doğru formatta verin. Örn: 20-80\n");
        return 1;
    }

    printf("IP: %s, Port Aralığı: %d-%d\n", ip, start_port, end_port);

    for (int port = start_port; port <= end_port; port++) {
        scan_port(ip, port);
    }

    return 0;
}