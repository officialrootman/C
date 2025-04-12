#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Renk kodları
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define RESET "\x1B[0m"

// Fonksiyon prototipleri
void banner();
void port_scanner();
void packet_sniffer();
void hash_generator();
void file_integrity_checker();

int main() {
    int choice;
    
    while(1) {
        banner();
        printf("\nSiber Güvenlik Multi-Tool Menüsü:\n");
        printf("1. Port Tarayıcı\n");
        printf("2. Paket Sniff Tool\n");
        printf("3. Hash Oluşturucu\n");
        printf("4. Dosya Bütünlük Kontrolü\n");
        printf("5. Çıkış\n");
        printf("\nSeçiminiz (1-5): ");
        scanf("%d", &choice);

        switch(choice) {
            case 1:
                port_scanner();
                break;
            case 2:
                packet_sniffer();
                break;
            case 3:
                hash_generator();
                break;
            case 4:
                file_integrity_checker();
                break;
            case 5:
                printf(KGRN "\nProgramdan çıkılıyor...\n" RESET);
                exit(0);
            default:
                printf(KRED "\nGeçersiz seçim!\n" RESET);
        }
    }
    return 0;
}

void banner() {
    system("clear"); // Linux için
    printf(KYEL);
    printf("╔══════════════════════════════════════╗\n");
    printf("║     Siber Güvenlik Multi-Tool v1.0   ║\n");
    printf("║     Geliştirici: @officialrootman    ║\n");
    printf("║     Tarih: 2025-04-12 15:07:06       ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf(RESET);
}

void port_scanner() {
    char target[100];
    int port, sock, result;
    struct sockaddr_in addr;
    
    printf("\nHedef IP: ");
    scanf("%s", target);
    
    printf("\nPort taraması başlatılıyor...\n");
    
    for(port = 1; port <= 1024; port++) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(target);
        
        result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        
        if(result == 0) {
            printf(KGRN "Port %d: AÇIK\n" RESET, port);
            close(sock);
        }
        close(sock);
    }
}

void packet_sniffer() {
    printf(KYEL "\n[*] Paket sniffing başlatılıyor...\n");
    printf("[*] Bu özellik root yetkisi gerektirir.\n");
    printf("[*] Şu anda geliştirme aşamasında...\n" RESET);
}

void hash_generator() {
    char text[1024];
    printf("\nHash'lenecek metni girin: ");
    scanf(" %[^\n]s", text);
    
    // Basit bir hash algoritması (örnek amaçlı)
    unsigned long hash = 5381;
    int c;
    char *str = text;
    
    while (c = *str++)
        hash = ((hash << 5) + hash) + c;
    
    printf(KGRN "\nHash değeri: %lu\n" RESET, hash);
}

void file_integrity_checker() {
    char filename[256];
    printf("\nKontrol edilecek dosya adı: ");
    scanf("%s", filename);
    
    FILE *file = fopen(filename, "rb");
    if(file == NULL) {
        printf(KRED "Dosya açılamadı!\n" RESET);
        return;
    }
    
    // Basit bir checksum hesaplama
    unsigned char checksum = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        checksum ^= ch;
    }
    
    printf(KGRN "\nDosya checksum değeri: %02x\n" RESET, checksum);
    fclose(file);
}