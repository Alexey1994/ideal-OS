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


;C H S   LBA
;0 0 1   0
;...
;0 0 18  17
;0 1 1   18
;...
;0 1 18  35
;1 0 1   36
;...
;1 0 18  53
;1 1 1   54
;...
;1 1 18  71
;1 1 1   72
;...

;C = LBA / (2 * 18)
;H = (LBA / 18) % 2
;S = (LBA % 18) + 1


;CX =     CH              CL
;cylinder 7 6 5 4 3 2 1 0 9 8
;sector                       5 4 3 2 1 0

;DH = head


read_sector:
	mov BX, 0x500

	xor DX, DX
	mov AX, [start_sector]
	mov CX, 18
	div CX
	mov CX, DX
	inc CX
	and CX, 0b00111111 ;sector
	mov DX, AX
	and DX, 1
	mov DH, DL ;head
	shr AX, 1
	mov CH, AL ;cylinder low
	shl AX, 6
	and AH, 0b11000000
	or CL, AH ;cylinder high

	mov DL, [drive_number]

	mov AX, 0x0201
	int 0x13
	jc read_error
	ret


write_sector:
	mov BX, 0x500

	xor DX, DX
	mov AX, [start_sector]
	mov CX, 18
	div CX
	mov CX, DX
	inc CX
	and CX, 0b00111111 ;sector
	mov DX, AX
	and DX, 1
	mov DH, DL ;head
	shr AX, 1
	mov CH, AL ;cylinder low
	shl AX, 6
	and AH, 0b11000000
	or CL, AH ;cylinder high

	mov DL, [drive_number]

	mov AX, 0x0301
	int 0x13
	jc read_error
	ret


start_sector: dd 1

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