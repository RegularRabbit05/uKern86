.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    cli
	mov $stack_top, %esp
	call kernel_main
	cli
1:	hlt
	jmp 1b

.size _start, . - _start
