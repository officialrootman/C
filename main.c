/* kernel.c */
void kernel_main(void) {
    const char *message = "Hello, World from C Kernel!";
    char *video_memory = (char *)0xb8000;  // VGA text modu başlangıcı

    // VGA ekranına yazdırma (her karakterin ardından bir attribute byte ekleniyor)
    for (int i = 0; message[i] != '\0'; i++) {
        video_memory[i * 2] = message[i];
        video_memory[i * 2 + 1] = 0x07;  // Standart beyaz- siyah renk
    }

    while (1);  // Sonsuz döngü, kernelin çalışmaya devam etmesini sağlar
}