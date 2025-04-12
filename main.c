#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>

#define PORT 8080       // Dinlenecek port numarası (gerektiğinde değiştirilebilir)
#define BUFFER_SIZE 1024

// Programın düzgün kapanması için global bayrak
volatile sig_atomic_t running = 1;

// SIGINT yakalayıcı: Ctrl+C ile çalışmayı sonlandırıyoruz.
void handle_sigint(int signum) {
    running = 0;
    printf("\nSIGINT alındı. Honeypot kapatılıyor...\n");
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address, client_addr;
    int addrlen = sizeof(address);

    // SIGINT (Ctrl+C) sinyalini yakala
    signal(SIGINT, handle_sigint);

    // 1. Socket oluşturulması (TCP)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket oluşturulurken hata oluştu");
        exit(EXIT_FAILURE);
    }

    // 2. Socket seçenekleri: Adresin tekrar kullanılabilir olması
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt hatası");
        exit(EXIT_FAILURE);
    }

    // 3. Adres ve port yapılandırması
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 4. Bind: Socket'i belirtilen IP ve port'a bağlama
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind işlemi başarısız");
        exit(EXIT_FAILURE);
    }

    printf("Honeypot port %d üzerinde çalışıyor. Bağlantılar bekleniyor...\n", PORT);

    // 5. Dinleme: Gelen bağlantıları kabul etme
    if (listen(server_fd, 3) < 0) {
        perror("Dinleme sırasında hata");
        exit(EXIT_FAILURE);
    }

    // 6. Sonsuz döngü: Gelen bağlantıları kabul et ve terminale yazdır
    while(running) {
        socklen_t client_addr_len = sizeof(client_addr);
        new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (new_socket < 0) {
            // Eğer running bayrağı sıfırlanmışsa (Ctrl+C) döngüden çık.
            if (!running)
                break;
            perror("Bağlantı kabul edilemedi");
            continue;
        }

        // Bağlantı anındaki zamanı al ve formatla
        time_t now = time(NULL);
        char time_str[26];
        ctime_r(&now, time_str);
        // Yeni satır karakterini kaldır
        size_t len = strlen(time_str);
        if (len > 0 && time_str[len - 1] == '\n')
            time_str[len - 1] = '\0';

        // Terminale bağlantı bilgisini yazdır
        printf("\n[%s] Bağlantı: %s\n", time_str, inet_ntoa(client_addr.sin_addr));

        // İstemciye hoşgeldiniz mesajı gönder
        const char *welcome_msg = "Honeypot'a hoş geldiniz!\n";
        send(new_socket, welcome_msg, strlen(welcome_msg), 0);

        // İstemciden gelen veriyi oku
        char buffer[BUFFER_SIZE] = {0};
        int bytes_read = read(new_socket, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            printf("Alınan veri: %s\n", buffer);
        } else if (bytes_read < 0) {
            perror("Veri okunurken hata oluştu");
        }

        // Bağlantıyı kapat
        close(new_socket);
    }

    // Server socket'ini kapat ve çık
    close(server_fd);
    printf("Honeypot kapatıldı.\n");
    return 0;
}
