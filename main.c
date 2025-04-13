#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 1024

int main() {
    char input[MAX_INPUT];
    char cwd[1024];

    printf("Basit Terminal Emülatörü\n");
    printf("Çıkmak için 'exit' yazın.\n");

    while (1) {
        // Geçerli dizini al
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s > ", cwd);
        } else {
            perror("getcwd");
            return 1;
        }

        // Kullanıcıdan input al
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
        int ret = system(input);
        if (ret == -1) {
            perror("Komut yürütüleme hatası");
        }
    }

    return 0;
}