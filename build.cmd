@echo off
echo Building Kernel...
compiler\bin\i686-elf-as boot.s -o out\boot.o
compiler\bin\i686-elf-gcc -m32 -g -c src\kernel.c -o out\kernel.o -std=gnu99 -ffreestanding -fomit-frame-pointer -Wall -Wextra
rem -Os 
compiler\bin\i686-elf-gcc -m32 -fpic -Wl,--oformat=binary -T linker.ld -o ..\File1 -ffreestanding -nostdlib -fomit-frame-pointer out\boot.o out\kernel.o -lgcc
echo Build completed!
@echo on