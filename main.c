#include <stdio.h>
#include <stdlib.h>

#pragma pack(push, 1)
typedef struct {
    unsigned short e_magic;    // Magic number "MZ" - 0x5A4D
    unsigned short e_cblp;
    unsigned short e_cp;
    unsigned short e_crlc;
    unsigned short e_cparhdr;
    unsigned short e_minalloc;
    unsigned short e_maxalloc;
    unsigned short e_ss;
    unsigned short e_sp;
    unsigned short e_csum;
    unsigned short e_ip;
    unsigned short e_cs;
    unsigned short e_lfarlc;
    unsigned short e_ovno;
    unsigned short e_res[4];
    unsigned short e_oemid;
    unsigned short e_oeminfo;
    unsigned short e_res2[10];
    long           e_lfanew;   // Yeni exe header’ın dosya içindeki adresi
} DOS_HEADER;
#pragma pack(pop)

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: %s <dos_executable_file>\n", argv[0]);
        return 1;
    }
  
    FILE *file = fopen(argv[1], "rb");
    if(file == NULL) {
        perror("Dosya açılamadı");
        return 1;
    }
  
    DOS_HEADER dosHeader;
    size_t bytesRead = fread(&dosHeader, 1, sizeof(DOS_HEADER), file);
    if(bytesRead != sizeof(DOS_HEADER)) {
        printf("Dosya okunurken hata oluştu.\n");
        fclose(file);
        return 1;
    }
  
    if(dosHeader.e_magic != 0x5A4D) {  // 'MZ' imzası: 0x4D 0x5A (little-endian)
        printf("Geçerli bir DOS MZ dosyası değil.\n");
    } else {
        printf("DOS MZ header bulundu!\n");
        printf("e_lfanew: %ld\n", dosHeader.e_lfanew);
    }
  
    fclose(file);
    return 0;
}
