#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080

// Rastgele sahte hata mesajları
const char *fake_errors[] = {
    "Sunucu hatası: Yetersiz bellek!",
    "Bağlantı zaman aşımına uğradı. Lütfen tekrar deneyin.",
    "Yetkilendirme başarısız. Geçersiz kimlik bilgileri.",
    "Veritabanı bağlantısı kurulamadı.",
    "502 Bad Gateway. Proxy sunucu hatası.",
    "Sistemde olağan dışı bir durum algılandı.",
};

// Rastgele gecikme oluşturma fonksiyonu
void random_delay() {
    int delay = rand() % 5 + 1; // 1 ile 5 saniye arasında rastgele gecikme
    sleep(delay);
}

// Sahte mesaj gönderme fonksiyonu
void send_fake_message(int socket) {
    int random_index = rand() % (sizeof(fake_errors) / sizeof(fake_errors[0]));
    const char *message = fake_errors[random_index];
    send(socket, message, strlen(message), 0);
    send(socket, "\n", 1, 0);
}

int main() {
    srand(time(NULL)); // Rastgelelik için zaman tabanlı seed

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

        // Rastgele gecikme
        random_delay();

        // Sahte hata mesajı gönder
        for (int i = 0; i < 3; i++) { // Saldırganı daha da yanıltmak için 3 farklı sahte mesaj
            send_fake_message(new_socket);
            random_delay();
        }

        // Bağlantıyı kapat
        close(new_socket);
    }

    return 0;
}