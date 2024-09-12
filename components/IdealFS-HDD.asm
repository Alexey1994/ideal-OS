;global addresses
%define kernel_address  0x8000
%define drive_number    0x7C00 - 1


org 0x7C00

mov AX, CS
mov DS, AX
mov SS, AX
mov ES, AX
mov SP, drive_number

mov [drive_number], DL
mov EBX, 0x500


read_file:
	call read_sector
	cmp word [BX], 0
	je kernel_not_found
	cmp word [BX], 1
	jne next_file
	mov SI, 0x500 + 16
	mov DI, kernel_name
	mov CX, 7
	rep cmpsb
	je kernel_found
next_file:
	mov EAX, [EBX + 4]
	cmp EAX, 0
	je kernel_not_found
	mov [start_sector], EAX
	jmp read_file


kernel_found:
	mov EAX, [EBX + 12]
	cmp EAX, 0
	je end_fs_read_sector
	mov [start_sector], EAX
	mov DI, kernel_address
	load_next_kernel_sector:
		call read_sector

		mov SI, 0x500 + 12
		mov CX, 500
		rep movsb

		mov EAX, [EBX + 4]
		cmp EAX, 0
		je end_fs_read_sector
		mov [start_sector], EAX
		jmp load_next_kernel_sector
	end_fs_read_sector:

push write_sector
push read_sector
push start_sector
jmp kernel_address


kernel_not_found:
	mov SI, kernel_not_found_message
	jmp handle_error


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


read_sector:
	mov DL, [drive_number]
	mov AH, 0x42
	mov SI, LBA_packet
	int 0x13
	jc read_error
	ret


write_sector:
	mov DL, [drive_number]
	mov AX, 0x4300
	mov SI, LBA_packet
	int 0x13
	jc read_error
	ret

	LBA_packet:
		size                   db 16
		zero                   db 0
		number_of_sectors      dw 1
		buffer_address_offset  dw 0x500
		buffer_address_segment dw 0
		start_sector           dq 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


read_error:
	mov SI, read_error_message
handle_error:
	call print_string
	
	xor AX, AX
	int 0x16
	int 0x19


;in  SI - printed string
print_string:
	mov AH, 0x0E
print_next_string_char:
	lodsb
	cmp AL, 0
	je end_print_string
	int 0x10
	jmp print_next_string_char
end_print_string:
	ret


kernel_not_found_message: db "no "
kernel_name:db "kernel", 0
read_error_message: db "read error", 0