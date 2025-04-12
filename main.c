#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 8080       // Dinlenecek port numarası (değiştirilebilir)
#define BUFFER_SIZE 1024

// Gelen bağlantı bilgilerini log dosyasına kaydeden fonksiyon
void log_connection(struct sockaddr_in *client_addr) {
    FILE *fp = fopen("honeypot.log", "a");
    if (fp == NULL) {
        perror("Log dosyası açılamadı");
        return;
    }
    // Şu anki zamanı al
    time_t now = time(NULL);
    char *timestr = ctime(&now);
    // Yeni satır karakterini kaldır
    if (timestr[strlen(timestr)-1] == '\n')
        timestr[strlen(timestr)-1] = '\0';

    fprintf(fp, "Bağlantı: %s - Zaman: %s\n", inet_ntoa(client_addr->sin_addr), timestr);
    fclose(fp);
}

int main(int argc, char *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *message = "Honeypot'a hoşgeldiniz!\n";

    // 1. Socket oluşturulması
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket oluşturulamadı");
        exit(EXIT_FAILURE);
    }

    // 2. Socket seçeneklerinin ayarlanması: Adresin yeniden kullanılabilir olması
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt hatası");
        exit(EXIT_FAILURE);
    }

    // 3. Adres yapılandırması
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 4. Bind işlemi: Socket'i belirlenen IP ve port'a bağlama
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind hatası");
        exit(EXIT_FAILURE);
    }
    printf("Honeypot %d portu üzerinde dinleniyor...\n", PORT);

    // 5. Dinleme: Gelen bağlantıları bekleme
    if (listen(server_fd, 3) < 0) {
        perror("Listen hatası");
        exit(EXIT_FAILURE);
    }

    // 6. Sürekli olarak gelen bağlantıları kabul etme
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("Accept hatası");
            continue;
        }

        // Gelen bağlantı loglanıyor
        log_connection(&client_addr);

        // Saldırganın dikkatini çekmek için hoşgeldiniz mesajı gönderiliyor
        send(new_socket, message, strlen(message), 0);

        // İsteğe bağlı olarak, gönderilen veriyi okuyabiliriz
        int bytes_read = read(new_socket, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            printf("Gelen veri: %s\n", buffer);
        }

        // Bağlantı kapatılıyor
        close(new_socket);
    }

    return 0;
}
