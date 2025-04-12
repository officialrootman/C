#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LOG_FILE "honeypot.log"
#define PORT 22  // SSH port'unu simüle ediyoruz

void log_attempt(const char* ip, const char* timestamp) {
    FILE* log = fopen(LOG_FILE, "a");
    if (log != NULL) {
        fprintf(log, "[%s] Bağlantı denemesi - IP: %s\n", timestamp, ip);
        fclose(log);
    }
}

char* get_timestamp() {
    time_t now = time(NULL);
    return strtok(ctime(&now), "\n");
}

int main() {
    int server_fd, client_fd;
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
    
    // Socket'i bağla
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind başarısız");
        exit(EXIT_FAILURE);
    }
    
    // Dinlemeye başla
    if (listen(server_fd, 10) < 0) {
        perror("Listen başarısız");
        exit(EXIT_FAILURE);
    }
    
    printf("Honeypot %d portunda çalışıyor...\n", PORT);
    
    while(1) {
        // Bağlantıları kabul et
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept başarısız");
            continue;
        }
        
        // Bağlanan IP'yi al
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        
        // Zaman damgasını al
        char* timestamp = get_timestamp();
        
        // Bağlantı denemesini kaydet
        log_attempt(client_ip, timestamp);
        
        printf("Bağlantı algılandı - IP: %s\n", client_ip);
        
        // Sahte SSH banner gönder
        char banner[] = "SSH-2.0-OpenSSH_8.2p1 Ubuntu-4ubuntu0.2\n";
        send(client_fd, banner, strlen(banner), 0);
        
        // Bağlantıyı kapat
        close(client_fd);
    }
    
    return 0;
}