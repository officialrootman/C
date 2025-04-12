#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

// Örnek imza
const char *virüs_imzası = "5d41402abc4b2a76b9719d911017c592"; // "hello" için md5 hash'i

void dosya_tara(const char *dosya_adı) {
    FILE *dosya = fopen(dosya_adı, "rb");
    if (!dosya) {
        printf("Dosya açılamadı: %s\n", dosya_adı);
        return;
    }

    unsigned char buffer[1024];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    size_t okunan;
    while ((okunan = fread(buffer, 1, sizeof(buffer), dosya)) > 0) {
        SHA256_Update(&sha256, buffer, okunan);
    }

    SHA256_Final(hash, &sha256);
    fclose(dosya);

    char hash_string[SHA256_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hash_string[i * 2], "%02x", hash[i]);
    }
    hash_string[SHA256_DIGEST_LENGTH * 2] = '\0';

    printf("Dosya Hash'i: %s\n", hash_string);

    if (strcmp(hash_string, virüs_imzası) == 0) {
        printf("Virüs Tespit Edildi!\n");
    } else {
        printf("Dosya Temiz.\n");
    }
}

int main() {
    const char *dosya_adı = "test.txt";
    dosya_tara(dosya_adı);
    return 0;
}