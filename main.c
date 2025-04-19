#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IOS
#define PLATFORM_IOS
#endif
#endif

// XOR şifreleme fonksiyonu
void xor_encrypt_decrypt(char *input, char *key, char *output) {
    size_t len = strlen(input);
    size_t key_len = strlen(key);
    for (size_t i = 0; i < len; i++) {
        output[i] = input[i] ^ key[i % key_len];
    }
    output[len] = '\0';
}

// Güvenli girdi okuma
void secure_input(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) {
        fprintf(stderr, "Girdi hatası.\n");
        exit(EXIT_FAILURE);
    }
    buffer[strcspn(buffer, "\n")] = '\0'; // Yeni satır karakterini kaldır
}

int main() {
    char input[256], key[256], encrypted[256], decrypted[256];

    printf("Metin girin: ");
    secure_input(input, sizeof(input));

    printf("Şifreleme anahtarı girin: ");
    secure_input(key, sizeof(key));

    xor_encrypt_decrypt(input, key, encrypted);
    printf("Şifrelenmiş metin: %s\n", encrypted);

    xor_encrypt_decrypt(encrypted, key, decrypted);
    printf("Çözülmüş metin: %s\n", decrypted);

#ifdef PLATFORM_IOS
    printf("Platform: iOS\n");
#else
    printf("Platform: Shell veya Diğer\n");
#endif

    return 0;
}