#include <stdio.h>
#include <string.h>

#define SIGNATURE "malware_signature"

int check_file_for_malware(const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Dosya açılamadı");
        return -1;
    }

    char buffer[256];
    while (fread(buffer, 1, sizeof(buffer), file)) {
        if (strstr(buffer, SIGNATURE)) {
            fclose(file);
            return 1; // Virüs bulundu
        }
    }

    fclose(file);
    return 0; // Temiz
}

int main() {
    const char *file_to_scan = "testfile.txt";
    int result = check_file_for_malware(file_to_scan);

    if (result == 1) {
        printf("Virüs bulundu: %s\n", file_to_scan);
    } else if (result == 0) {
        printf("Dosya temiz: %s\n", file_to_scan);
    } else {
        printf("Tarama sırasında bir hata oluştu.\n");
    }

    return 0;
}