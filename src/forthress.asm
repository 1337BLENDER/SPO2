global _start

%include "macro.inc"
%include "util.inc"

%define pc r15
%define w r14
%define rstack r13

section .bss
resq 1023 				;return stack
rstack_start: resq 1
input_buf: resb 1024	;buffer to read words into
user_dict: resq 65536	;memory for used-defined words
state: resq 1			;state. 0 - interpretation 1 - compilation
user_mem: resq 65536	;memory that user can use

section .rodata
msg_not_a_word: db " is not a Forth word or number!", 10, 0

section .text
%include "words.inc"

section .data
last_word: dq _lw
here: dq user_dict
dp: dq user_mem

section .text
next:					;fetch next word
	mov w, [pc]
	add pc, 8
	jmp [w]

_start: 
	jmp i_init
	
