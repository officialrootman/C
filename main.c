#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 1024

void execute_command(char *input) {
    // Eğer kullanıcı 'git clone' komutunu girdiyse
    if (strncmp(input, "git clone", 9) == 0) {
        printf("[INFO] Git klonlama işlemi başlatılıyor...\n");
        int ret = system(input);
        if (ret == -1) {
            perror("Git klonlama hatası");
        }
        return;
    }

    // Eğer kullanıcı 'pkg' komutunu girdiyse
    if (strncmp(input, "pkg", 3) == 0) {
        printf("[INFO] Paket yönetim komutu algılandı: %s\n", input);
        int ret = system(input);
        if (ret == -1) {
            perror("Paket yönetim komutu hatası");
        }
        return;
    }

    // Diğer tüm komutlar için
    int ret = system(input);
    if (ret == -1) {
        perror("Komut yürütüleme hatası");
    }
}

int main() {
    char input[MAX_INPUT];
    char cwd[1024];

    printf("Gelişmiş Terminal Emülatörü\n");
    printf("Çıkmak için 'exit' yazın.\n");

    while (1) {
        // Geçerli dizini al
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s > ", cwd);
        } else {
            perror("getcwd");
            return 1;
        }

        // Kullanıcıdan giriş al
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break;
        }

        // Yeni satır karakterini kaldır
        input[strcspn(input, "\n")] = 0;

        // 'exit' komutunu kontrol et
        if (strcmp(input, "exit") == 0) {
            printf("Çıkılıyor...\n");
            break;
        }

        // Komutları çalıştır
        execute_command(input);
    }

    return 0;
}