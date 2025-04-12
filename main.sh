# Bootloader derlemesi (nasm kullanılarak)
nasm -f bin boot.s -o boot.bin

# Kernel C kodunun derlenmesi
i686-elf-gcc -m32 -ffreestanding -c kernel.c -o kernel.o

# Kernel için linker aşaması
i686-elf-ld -T linker.ld -o kernel.bin kernel.o

# Bootloader ve kernel'ı tek dosya halinde birleştirme
cat boot.bin kernel.bin > os-image.bin

# QEMU ile test etme
qemu-system-i386 -drive format=raw,file=os-image.bin