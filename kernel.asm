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
extern kmain			;defined in the C file

start:
	cli			;block interrupts
	mov esp, stack_space	;set stack pointer
	call kmain		;call kmain C function
	hlt			;halt cpu

section .bss
	resb 8192		;8kb for stack
stack_space:
;;	lol!
