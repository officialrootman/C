#include <stdlib.h>

int main() {
    // apk add python komutunu çalıştır
    int result = system("apk add python3");

    // Komutun başarılı olup olmadığını kontrol et
    if (result == 0) {
        printf("Python3 başarıyla yüklendi.\n");
    } else {
        printf("Bir hata oluştu.\n");
    }

    return 0;
}