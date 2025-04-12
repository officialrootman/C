#include <stdio.h>
#include <string.h>

#define TARGET_PASSWORD "abc"  // Tahmin edilmesi gereken şifre
#define MAX_LENGTH 3           // Şifrenin maksimum uzunluğu

// Karakter seti (örneğin, küçük harfler)
const char charset[] = "abcdefghijklmnopqrstuvwxyz";

// Bruteforce fonksiyonu
void brute_force(char *attempt, int position, int max_length) {
    if (position == max_length) {
        // Şifrenin sonuna ulaşıldı, kontrol et
        attempt[position] = '\0'; // Null terminator ekle
        if (strcmp(attempt, TARGET_PASSWORD) == 0) {
            printf("Şifre bulundu: %s\n", attempt);
        }
        return;
    }

    // Karakter setindeki her karakteri dene
    for (int i = 0; i < strlen(charset); i++) {
        attempt[position] = charset[i];
        brute_force(attempt, position + 1, max_length); // Rekürsif deneme
    }
}

int main() {
    char attempt[MAX_LENGTH + 1]; // Şifre denemesi için buffer
    printf("Brute force başlıyor...\n");

    // Şifrenin tüm uzunluklarını dene (1'den MAX_LENGTH'e kadar)
    for (int length = 1; length <= MAX_LENGTH; length++) {
        brute_force(attempt, 0, length);
    }

    printf("Brute force tamamlandı.\n");
    return 0;
}