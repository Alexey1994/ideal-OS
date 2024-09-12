org 0x8000

;push BP
mov BP, SP

;start_sector [BP]
;read_sector  [BP + 2]
;write_sector [BP + 4]


mov [old_SP], SP
mov [old_BP], BP


;enable A20
in AL, 0x92
or AL, 2
out 0x92, AL


call setup_video


;get_memory_region
push ES

xor DI, DI
xor EBX, EBX
next_memory_region:
	mov AX, 0x2000
	mov ES, AX
	mov ECX, 20
	mov EDX, 0x534D4150 ;'SMAP'
	mov EAX, 0xE820
	int 0x15
	jc get_memory_region_error
	cmp EAX, 0x534D4150 ;'SMAP'
	jne get_memory_region_error
	cmp CX, 20
	jne get_memory_region_error
	add DI, 20
	cmp EBX, 0
	jne next_memory_region
get_memory_region_error:

mov [size_of_memory_regions], DI
pop ES


lgdt [GDT_pointer]


call switch_to_32_bit
use32
mov ESP, 0x80000
mov EBP, ESP


push 0x20000
push dword [size_of_memory_regions]
push VESA_Mode
push write_sector
push read_sector
push interrupt_queue
call kernel_main


call switch_to_16_bit
use16
mov SP, [old_SP]

mov AX, 0x0003
int 0x10

mov BX, no_actions_message
call print_string

no_actions:
	xor AX, AX
	int 16h
	int 19h

no_actions_message: db 10, 13, "IdealOS ended. Press any key to reboot.", 0


size_of_memory_regions: dd 0






use16


VESA: times 512 db 0
%define VESA__video_modes 14

VESA_Mode: times 256 db 0
%define VESA_Mode__pitch  16
%define VESA_Mode__width  18
%define VESA_Mode__height 20
%define VESA_Mode__bpp    25

VESA_CRTC: times 512 db 0


setup_video:
	mov AX, 0x4F00
	mov DI, VESA
	int 0x10
	cmp AX, 0x004F
	jne setup_VGA

	mov SI, VESA
	mov EBX, [SI + VESA__video_modes]

test_VESA_Mode:
	mov AX, 0x4F01
	mov CX, [EBX]
	cmp CX, 0xFFFF
	je VESA_Mode_not_found
	mov DI, VESA_Mode
	int 0x10

	;push CX

	;mov AX, [DI + VESA_Mode__width]
	;call print_number
	;mov AL, 'x'
	;call print_char

	;mov AX, [DI + VESA_Mode__height]
	;call print_number
	;mov AL, 'x'
	;call print_char

	;xor AX, AX
	;mov AL, [DI + VESA_Mode__bpp]
	;call print_number
	;mov AL, 10
	;call print_char
	;mov AL, 13
	;call print_char

	;pop CX


	cmp word [DI + VESA_Mode__width], 640
	jne next_VESA_Mode
	cmp word [DI + VESA_Mode__height], 480
	jne next_VESA_Mode
	cmp byte [DI + VESA_Mode__bpp], 24
	jl next_VESA_Mode


	mov AX, 0x4F02
	mov DI, VESA_CRTC
	mov BX, CX
	;or BX, 0x4000
	int 0x10

	ret

next_VESA_Mode:
	add EBX, 2
	jmp test_VESA_Mode

	ret

VESA_Mode_not_found:
	;call print_string
	;jmp $
	jmp setup_VGA

setup_VGA:
	mov AX, 0x0012 ;640x480x16
	int 0x10
	ret


;in  AL - char code
print_char:
	mov AH, 0x0E
	int 0x10
	ret


;in  BX - printed null terminated string
print_string:
	mov AL, [BX]
	cmp AL, 0
	je end_print_string
	call print_char
	inc BX
	jmp print_string
	
	end_print_string:
	ret


;in  AX - printed number
print_number:
	mov CX, 10
	mov DX, 0
	div CX
	push DX
	cmp AX, 0
	je skip_print_number
	call print_number
skip_print_number:
	pop DX
	add DX, '0'
	mov AL, DL
	mov AH, 0x0E
	int 0x10
	ret






use32


old_SP: dw 0
old_BP: dw 0
old_ESP: dd 0
old_EBP: dd 0
sector_number: dd 0


read_sector:
	mov EAX, [ESP + 4]
	mov [sector_number], EAX
	
	mov [old_ESP], ESP
	mov [old_EBP], EBP
	call switch_to_16_bit
	use16
	mov SP, [old_SP]
	mov BP, [old_BP]

	mov EAX, [sector_number]
	xor EBX, EBX
	mov BX, [BP]
	mov [EBX], EAX
	mov AX, [BP + 2]
	call AX
	
	call switch_to_32_bit
	use32
	mov ESP, [old_ESP]
	mov EBP, [old_EBP]
	
	ret


write_sector:
	mov EAX, [ESP + 4]
	mov [sector_number], EAX
	
	mov [old_ESP], ESP
	mov [old_EBP], EBP
	call switch_to_16_bit
	use16
	mov SP, [old_SP]
	mov BP, [old_BP]

	mov EAX, [sector_number]
	xor EBX, EBX
	mov BX, [BP]
	mov [EBX], EAX
	mov AX, [BP + 4]
	call AX
	
	call switch_to_32_bit
	use32
	mov ESP, [old_ESP]
	mov EBP, [old_EBP]
	
	ret
	






%macro out_8 2
	mov AL, %2
	out %1, AL
%endmacro


;BX - PIC setting, 0x0870 for 16 bit, 0x2028 for 32
use16
reset_pic:
	out_8 0x20, 0x11
	out_8 0x21, BH
	out_8 0x21, 0x04
	out_8 0x21, 0x01
	out_8 0x21, 0x00
	
	out_8 0xA0, 0x11
	out_8 0xA1, BL
	out_8 0xA1, 0x02
	out_8 0xA1, 0x01
	out_8 0xA1, 0x00

	ret


return_EIP: dd 0

use16
switch_to_32_bit:
	cli
	
	pop word[return_EIP]
	
	mov BX, 0x2028
	call reset_pic
	
	lidt [IDT_pointer]

	mov EAX, CR0
	or EAX, 1
	mov CR0, EAX
	
	jmp 8:switch_to_32_bit_1

	use32
switch_to_32_bit_1:
	mov EAX, 16
	mov DS, EAX
	mov SS, EAX
	mov ES, EAX
	mov FS, EAX
	mov GS, EAX
	
	sti
	
	mov EAX, [return_EIP]
	jmp EAX


align 4
idtr_16:
	dw 0x3FF
	dd 0

use32
switch_to_16_bit:
	cli
	
	pop dword[return_EIP]
	
	lidt [idtr_16]

	mov AX, 32
	mov DS, AX
	mov SS, AX
	mov ES, AX
	mov FS, AX
	mov GS, AX

	jmp 24:switch_to_16_bit_1
	
	align 16
	use16
switch_to_16_bit_1:

	mov EAX, CR0
	and EAX, 0xFFFFFFFE
	mov CR0, EAX

	jmp 0:switch_to_16_bit_2
	
switch_to_16_bit_2:
	mov AX, CS
	mov DS, AX
	mov SS, AX
	mov ES, AX
	mov FS, AX
	mov GS, AX
	
	mov BX, 0x0870
	call reset_pic
	
	sti
	
	mov AX, [return_EIP]
	jmp AX






%define INTERRUPT_QUEUE_SIZE 256

interrupt_queue:
interrupt_queue_start: dd 0
interrupt_queue_end: dd 0

interrupt_queue_data:
	times INTERRUPT_QUEUE_SIZE db 0

%macro interrupt_handler 1
	interrupt_handler_%1:
		push EAX
		mov AL, %1
		jmp interrupt_handler_main
%endmacro

%macro interrupt_handler_pic1 1
	interrupt_handler_%1:
		push EAX
		mov AL, %1
		jmp interrupt_handler_pic1_main
%endmacro

%macro interrupt_handler_pic2 1
	interrupt_handler_%1:
		push EAX
		mov AL, %1
		jmp interrupt_handler_pic2_main
%endmacro

use32

;%assign i 0
;%rep 256
;interrupt_handler i
;%assign i i+1
;%endrep

%assign i 0
%rep 32
interrupt_handler i
%assign i i+1
%endrep

%assign i 32
%rep 8
interrupt_handler_pic1 i
%assign i i+1
%endrep

%assign i 40
%rep 8
interrupt_handler_pic2 i
%assign i i+1
%endrep

%assign i 48
%rep 208
interrupt_handler i
%assign i i+1
%endrep

interrupt_handler_main:
	push EBX
	push ECX

	mov EBX, [interrupt_queue_end]
	mov ECX, EBX
	
	inc ECX
	cmp ECX, INTERRUPT_QUEUE_SIZE
	jne skip_rewind_to_queue_start
	xor ECX, ECX
	skip_rewind_to_queue_start:
	cmp ECX, [interrupt_queue_start]
	je interrupt_queue_filled_error_handler
	
	add EBX, interrupt_queue_data
	mov [EBX], AL
	
	mov [interrupt_queue_end], ECX

	pop ECX
	pop EBX
	pop EAX
	
	iret

interrupt_handler_pic1_main:
	push EBX
	push ECX

	mov EBX, [interrupt_queue_end]
	mov ECX, EBX
	
	inc ECX
	cmp ECX, INTERRUPT_QUEUE_SIZE
	jne skip_rewind_to_queue_start1
	xor ECX, ECX
skip_rewind_to_queue_start1:
	cmp ECX, [interrupt_queue_start]
	je interrupt_queue_filled_error_handler
	
	add EBX, interrupt_queue_data
	mov [EBX], AL
	
	mov [interrupt_queue_end], ECX
	
	mov AL, 0x20
	out 0x20, AL

	pop ECX
	pop EBX
	pop EAX
	
	iret


interrupt_handler_pic2_main:
	push EBX
	push ECX

	mov EBX, [interrupt_queue_end]
	mov ECX, EBX
	
	inc ECX
	cmp ECX, INTERRUPT_QUEUE_SIZE
	jne skip_rewind_to_queue_start2
	xor ECX, ECX
skip_rewind_to_queue_start2:
	cmp ECX, [interrupt_queue_start]
	je interrupt_queue_filled_error_handler
	
	add EBX, interrupt_queue_data
	mov [EBX], AL
	
	mov [interrupt_queue_end], ECX
	
	mov AL, 0x20
	out 0x20, AL
	out 0xA0, AL

	pop ECX
	pop EBX
	pop EAX
	
	iret


interrupt_queue_filled_error_handler:
	call switch_to_16_bit
	use16
	mov SP, [old_SP]
	mov BP, [old_BP]

	mov AX, 0x0003
	int 0x10

	xor BX, BX
print_next_interrupt:
	xor AX, AX
	mov AL, [BX + interrupt_queue_data]
	call print_number
	mov AL, ' '
	call print_char

	inc BX
	cmp BX, INTERRUPT_QUEUE_SIZE
	jne print_next_interrupt
	
	mov BX, interrupt_queue_filled_message
	call print_string

	jmp no_actions

interrupt_queue_filled_message: db "ERROR: interrupt queue filled", 10, 0






align 16
GDT:
	; dummy
	dq 0

	; CODE (CS register = 8)
	dw 0xFFFF     ; размер сегмента
	dw 0          ; базовый адрес
	db 0          ; базовый адрес
	db 0b10011010 ; 1    сегмент правильный(должно быть 1)
	              ; 00   уровень привилегий(меньше - больше привилегий)
	              ; 1    если сегмент в памяти то 1
	              ; 1    сегмент исполняемый
	              ; 0    направление для сегмента данных либо возможность перехода с низких привилегий на высокие для сегмента кода(1 - разрешено, 0 - запрещено)
	              ; 1    разрешение на чтение для сегмента кода, разрешение на запись для сегмента данных
	              ; 0    бит доступа к сегменту, устанавливается процессором(рекомендуется 0)
	db 0b11001111 ; 1    гранулярность(если 0, то размер адреса равен размеру сегмента кода, если 1 то размеру сегмента кода * 4096)
	              ; 1    размер, если 0 и 64 битный режим(следующий бит) = 0, то селектор определяет 16 битный режим, если 1 - 32 битный. Если 64 битный режим равен 1, то должен быть равен 0(значение 1 зарезервировано, будет генерировать исключение)
	              ; 0    64 битный режим
	              ; 0    зарезервировано
	              ; 1111 размер сегмента
	db 0          ;      базовый адрес

	; DATA (DS register = 16)
	dw 0xffff     ; размер сегмента
	dw 0          ; базовый адрес
	db 0          ; базовый адрес
	db 0b10010010 ; 1    сегмент правильный(должно быть 1)
	              ; 00   уровень привилегий(меньше - больше привилегий)
	              ; 1    если сегмент в памяти то 1
	              ; 0    сегмент исполняемый
	              ; 0    направление для сегмента данных либо возможность перехода с низких привилегий на высокие для сегмента кода
	              ; 1    разрешение на чтение для сегмента кода, разрешение на запись для сегмента данных
	              ; 0    бит доступа к сегменту, устанавливается процессором(рекомендуется 0)
	db 0b11001111 ; 1    гранулярность(если 0, то размер адреса равен размеру сегмента кода, если 1 то размеру сегмента кода * 4096)
	              ; 1    размер, если 0 и 64 битный режим(следующий бит) = 0, то селектор определяет 16 битный режим, если 1 - 32 битный. Если 64 битный режим равен 1, то должен быть равен 0(значение 1 зарезервировано, будет генерировать исключение)
	              ; 0    64 битный режим
	              ; 0    зарезервировано
	              ; 1111 размер сегмента
	db 0          ;      базовый адрес
	
	; CODE16 (CS register = 24)
	dw 0xFFFF     ; размер сегмента
	dw 0          ; базовый адрес
	db 0          ; базовый адрес
	db 0b10011010 ; 1    сегмент правильный(должно быть 1)
	              ; 00   уровень привилегий(меньше - больше привилегий)
	              ; 1    0 - системный, 1 - код или данные
	              ; 1    сегмент исполняемый
	              ; 1    направление для сегмента данных либо возможность перехода с низких привилегий на высокие для сегмента кода(1 - разрешено, 0 - запрещено)
	              ; 1    разрешение на чтение для сегмента кода, разрешение на запись для сегмента данных
	              ; 0    бит доступа к сегменту, устанавливается процессором(рекомендуется 0)
	db 0b00001111 ; 0    гранулярность(если 0, то размер адреса равен размеру сегмента кода, если 1 то размеру сегмента кода * 4096)
	              ; 0    размер, если 0 и 64 битный режим(следующий бит) = 0, то селектор определяет 16 битный режим, если 1 - 32 битный. Если 64 битный режим равен 1, то должен быть равен 0(значение 1 зарезервировано, будет генерировать исключение)
	              ; 0    64 битный режим
	              ; 0    зарезервировано
	              ; 1111 размер сегмента
	db 0          ;      базовый адрес

	; DATA16 (DS register = 32)
	dw 0xFFFF     ; размер сегмента
	dw 0          ; базовый адрес
	db 0          ; базовый адрес
	db 0b10010010 ; 1    сегмент правильный(должно быть 1)
	              ; 00   уровень привилегий(меньше - больше привилегий)
	              ; 1    0 - системный, 1 - код или данные
	              ; 0    сегмент исполняемый
	              ; 0    направление для сегмента данных либо возможность перехода с низких привилегий на высокие для сегмента кода
	              ; 1    разрешение на чтение для сегмента кода, разрешение на запись для сегмента данных
	              ; 0    бит доступа к сегменту, устанавливается процессором(рекомендуется 0)
	db 0b00001111 ; 0    гранулярность(если 0, то размер адреса равен размеру сегмента кода, если 1 то размеру сегмента кода * 4096)
	              ; 0    размер, если 0 и 64 битный режим(следующий бит) = 0, то селектор определяет 16 битный режим, если 1 - 32 битный. Если 64 битный режим равен 1, то должен быть равен 0(значение 1 зарезервировано, будет генерировать исключение)
	              ; 0    64 битный режим
	              ; 0    зарезервировано
	              ; 1111 размер сегмента
	db 0          ;      базовый адрес

GDT_pointer:
	dw $ - GDT ;размер
	dd GDT     ;адрес




%macro interrupt_descriptor 3
	dw %1  ; handler_address_low
	dw %2  ; selector
	db 0   ; zero
	db %3  ; attributes
	dw 0   ; handler_address_high
%endmacro

align 16
IDT:
	%assign i 0
	%rep 256
		interrupt_descriptor interrupt_handler_%[i], 8, 0x8E
	%assign i i+1
	%endrep
	

IDT_pointer:
	dw $ - IDT ;размер
	dd IDT     ;адрес


align 32
kernel_main: