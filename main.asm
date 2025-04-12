; boot.s - Basit bootloader örneği
[BITS 16]
[ORG 0x7C00]

start:
    ; Basit bir "Hello" yazdırmak için BIOS interrupt kullanımı
    mov ah, 0x0E      ; Teletype output için fonksiyon numarası
    mov si, hello_msg
print_char:
    lodsb             ; AL'e değer yükle
    cmp al, 0
    je hang_here
    int 0x10          ; Karakteri ekrana yazdır
    jmp print_char

hang_here:
    jmp hang_here     ; Sonsuz döngü

hello_msg db 'Hello, OS!', 0

; Boot sector un alt limiti olacak kadar bayt doldurma
times 510 - ($ - $$) db 0
dw 0xAA55           ; Boot sector bayrağı