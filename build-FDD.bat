@mkdir "bin/components"
@mkdir "bin/components/programs"
@mkdir "bin/components/libraries"

@nasm "components/IdealFS-FDD.asm" -o "bin/components/IdealFS" ^
	&& nasm "components/kernel/main16.asm" -o "bin/components/kernel16" ^
	&& tcc32 -w "-Icomponents/global" -nostdlib -c "components/kernel/main32.c" -o "bin/components/kernel32.elf" ^
	&& load "bin/components/kernel16" | to ld > "bin/components/kernel16.ld" ^
	&& ld -T kernel_script.ld -o "bin/components/kernel32.o" "bin/components/kernel32.elf" ^
	&& objcopy -O binary -S "bin/components/kernel32.o" "bin/components/kernel" ^
	&& build_program "libraries/graphics" ^
	&& build_program "libraries/ps2" ^
	&& build_program "programs/bootscreen" ^
	&& build_program "programs/shell" ^
	&& create.c.exe 144 > "bin/storage" ^
	&& create.c.exe IdealFS "bin/storage" "bin/components/IdealFS" ^
		"bin/components/kernel" "kernel" ^
		"components/kernel/main32.c" "kernel.c" ^
		"bin/components/libraries/graphics" "graphics.com" ^
		"bin/components/libraries/ps2" "ps2.com" ^
		"bin/components/programs/bootscreen" "bootscreen.com" ^
		"bin/components/programs/shell" "shell.com" ^
	&& qemu -usb -m 256 -no-reboot -fda "bin\storage" ^
	|| pause