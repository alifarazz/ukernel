;; kernel.asm
bits 32
section .text
	;multiboot spec
	align 4	
	dd 0x1BADB002			;magic
	dd 0x00				;flags
	dd -(0x1BADB002)	;checksum
	; magic + flag + checksum should be 0
	
global start
global read_port
global write_port
global load_idt
global keyboard_handler


;defined in the C file
extern kmain			
extern keyboard_handler_main
	
read_port:
	mov edx, [esp + 4]
	in al, dx
	ret

write_port:
	mov edx, [esp + 4]
	mov al,	 [esp + 8]
	out dx, al
	ret

load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	sti
	ret

keyboard_handler:
	call keyboard_handler_main
	iretd	
		
start:
	cli			;block interrupts
	mov esp, stack_space	;set stack pointer
	call kmain		;call kmain C function
	hlt			;halt cpu

section .bss
	resb 8192		;8kb for stack
stack_space:
;;	well...
