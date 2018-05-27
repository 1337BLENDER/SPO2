;Drop from data stack
;( a -- )
native "drop", drop
	add rsp, 8
	jmp next

;Swap first and second on stack
;( a b -- b a )
native "swap", swap
	pop rax
	pop rcx
	push rax
	push rcx
	jmp next

;Duplicate top of the stack
;( a -- a a )
native "dup", dup
	pop rax
	push rax
	push rax
	jmp next

;Rotate first three cell on the stack
;( a b c -- b c a )
native "rot", rot
	pop rcx
	pop rdx
	pop rax
	push rdx
	push rcx
	push rax
	jmp next

;Add contents of the first cell to the contents of the second cell
;( b a -- [ a + b ] )
native "+", add
	pop rax
	add [rsp], rax
	jmp next

;Multiply first and second cells
;( b a -- [ a * b ] )
native "*", mul
	pop rax
	pop rcx
	imul rcx
	push rax
	jmp next

;Divide first cell on stack to the second cell on stack
;( b a -- [ a / b ] )
native "/", div
	pop rax
	pop rcx
	cqo
	idiv rcx
	push rax
	jmp next

;Get modulo of the division of the first cell to the second cell on stack
;( b a -- [ a % b ] )
native "%", mod
	pop rax
	pop rcx
	cqo
	idiv rcx
	push rdx
	jmp next

;Subtract second cell onthe stack from the first one
;( b a -- [ a - b ] )
native "-", sub
	pop rax
	sub [rsp], rax
	jmp next


;Compare first and second cell
;( b a -- [ a < b ] )
native "<", less
	pop rax
	pop rcx
	cmp rax, rcx
	jl .less
	xor rax, rax
	jmp .end
	.less:
	mov rax, 1
	.end:
	push rax
	jmp next

;Logical inversion of TOS
;( a -- a' ) a' = 0 if a != 0 a' = 1 if a == 0
native "not", not
	pop rax
	test rax, rax
	jz .zero
	xor rax, rax
	jmp .end
	.zero:
	mov rax, 1
	.end:
	push rax
	jmp next
	
;Check if first cell on stack equals to the second one
;( a b -- c ) c = 1 if a == b c = 0 if a != b
native "=", equals
	pop rax
	pop rcx
	cmp rax, rcx
	jne .not_equal
	mov rax, 1
	jmp .end
	.not_equal:
	xor rax, rax
	.end:
	push rax
	jmp next

;Logical and between first and second cells on the stack
;( a b -- a && b ) Logical and
native "land", land
	pop rax
    pop rcx
    test rax, rax
    jz .false
    push rcx
    jmp next
	.false:
    push rax
    jmp next

;Logical or between first and second cells on the stack
;( a b -- a || b ) Logical or
native "lor", lor
	pop rax
    pop rcx
    test rax, rax
    jnz .true
    push rcx
    jmp next
	.true:
    push rax
    jmp next

;Bitwise and between first and second cells on the stack
;( a b -- a & b ) Bitwise and
native "and", and
	pop rax
	pop rcx
	and rax, rcx
	push rax
	jmp next

;Bitwise or between first and second cells on the stack
;( a b -- a | b ) Bitwise or
native "or", or
	pop rax
	pop rcx
	or rax, rcx
	push rax
	jmp next

;Drops element from stack and sends it to stdout
native ".", dot
	pop rdi
	call print_int
	jmp next

section .data
	st_base: dq 0

;Initialization
native "init", init
	mov rstack, rstack_start
	mov pc, main_stub
	mov qword [state], 0
	cmp qword [st_base], 0
	je .first
	mov rsp, [st_base]
	jmp next
	.first:
	mov [st_base], rsp
	jmp next

;Shows stack contents. Does not pop elements
native ".S", show
	mov rcx, rsp
	.loop:
	cmp rcx, [st_base]
	jae next
	mov rdi, [rcx]
	push rcx
	call print_int
	call print_newline
	pop rcx
	add rcx, 8
	jmp .loop

;Push from return stack into data stack
native "r>", from_r
	rpop rax
	push rax
	jmp next

;Pop from data stack into return stack
native ">r", to_r
	pop rax
	rpush rax
	jmp next

;Non-destructive copy from the top of return stack to the top of data stack
native "r@", cpy_r
	push qword [rstack]
	jmp next

;Outputs a single character to stdout
;( c -- )
native "emit", emit
	pop rdi
	call print_char
	jmp next

;Reads word from stdin and stores it starting at address addr. Word length is pushed into stack
;( addr -- len )
native "word", word
	pop rdi
	call read_word
	push rdx
	jmp next

;Parses an integer from string
;( str -- len num )
native "number", number
	pop rdi
	call parse_int
	push rdx
	push rax
	jmp next

; Prints a null-terminated string
;( addr -- )
native "prints", prints
	pop rdi
	call print_string
	jmp next

;Executes syscall.
;The following registers store arguments (according to ABI) rdi , rsi , rdx , r10 , r8 and r9
;( call_num a1 a2 a3 a4 a5 a6 -- new_rax new_rdx)
native "syscall", syscall
	pop r9
	pop r8
	pop r10
	pop rdx
	pop rsi
	pop rdi
	pop rax
	syscall
	push rax
	push rdx
	jmp next

;Fetch value from memory
;( addr -- value )
native "@", fetch
	pop rax
	push qword [rax]
	jmp next

;Store value by address
;( val addr -- )
native "!", store
	pop rcx
	pop rax
	mov [rcx], rax
	jmp next

;Read one byte starting at addr
;( addr -- char )
native "c@", fetch_b
	pop rax
	xor rcx, rcx
	mov cl, byte [rax]
	push rcx
	jmp next

;Store one byte by address
;( char addr -- )
native "c!", store_b
	pop rcx
	pop rax
	mov [rcx], al
	jmp next

;Accepts a null-terminated string, calculates its length
;( str -- len )
native "count", count
	pop rdi
	call string_length
	push rax
	jmp next

;Prints a certain amount of characters from string
;( str cnt -- )
native "printc", printc
	pop rcx
	xor rsi, rsi
	pop rax
	.loop:
	cmp rsi, rcx
	jae .end
	movzx rdi, byte [rax+rsi]
	test rdi, rdi
	jz .end
	push rax
	push rcx
	push rsi
	call print_char
	pop rsi
	pop rcx
	pop rax
	inc rsi
	jmp .loop
	.end:
	jmp next
	

;Exits Forthress
native "bye", bye
	mov rax, 60
	xor rdi, rdi
	syscall

;This is the implementation of any colon-word.
;The XT itself is not used, but the implementation (i_docol) is
native "docol", docol
	rpush pc
	add w, 8
	mov pc, w
	jmp next

;Exit from colon word.
native "exit", exit
	rpop pc
	jmp next

;Accepts a pointer to a string, returns pointer to the word header in dictionary
;( str -- header_addr )
native "find", find
	mov rsi, [last_word]
	mov rax, [state]
	test rax, rax
	jz .loop
	mov rsi, [rsi] 
;Skip currntly defining word to avoid word loops
	.loop:
	mov rdi, [rsp]
	push rsi
	add rsi, 9
	call string_equals
	pop rsi
	test rax, rax
	jnz .found
	mov rsi, [rsi]
	test rsi, rsi
	jnz .loop
	.word_not_found:
	mov qword [rsp], 0
	jmp next
	.found:
	mov [rsp], rsi
	jmp next

;Converts word header start address to the execution token
;( word_addr -- xt )
native "cfa", cfa
	pop rcx
	add rcx, 9
;Skip link to previos word and following 0

	.loop:
	mov al, [rcx]
	test al, al
	jz .end
	inc rcx
	jmp .loop
;Skip word name

	.end:
	add rcx, 2
;Skip null-terminator and flag bit

	push rcx
	jmp next
	
;Jump to a location. Location is absolute. Compile-only
native "branch", branch
	mov pc, [pc]
	jmp next

;Jump to a location if TOS = 0. Location is calculated in a similar way
;Compile-only
native "0branch", branch0
	pop rax
	test rax, rax
	jz .br
	add pc, 8
	jmp next
	.br:
	mov pc, [pc]
	jmp next

;Pushes a value immediately following this XT
native "lit", lit
	push qword [pc]
	;pc points at value following this XT
	add pc, 8
	jmp next

;Address of the input buffer (is used by interpreter/compiler)
const inbuf, input_buf

;Address of user memory
const mem, user_mem

;Header of last word address
const last_word, last_word

;State cell address.
;The state cell stores either 1 (compilation mode) or 0 (interpretation mode).
const state, state

;Points to the last cell of the word currently being defined
const here, [here]

;Execute word with this execution token on TOS
native "execute", execute
	pop rax
	mov w, rax ;move correct xt (xt of the command which needs to be executed) to w
	jmp [rax]  ;go to the word implementation

;Add x to the word being defined
;( x -- )
native ",", comma
	pop rax
	mov rcx, [here]
	mov [rcx], rax
	add rcx, 8
	mov [here], rcx
	jmp next

;Add a single byte to the word being defined
;( c -- )
native "c,", comma_b
	pop rax
	mov rcx, [here]
	mov [rcx], al
	add rcx, 1
	mov [here], rcx
	jmp next

;Create an entry in the dictionary name is the new name
;Only immediate flag is implemented ATM
;( flags name -- )
native "create", create
	mov rax, [here]
	mov rcx, [last_word]
	mov [rax], rcx
	mov [last_word], rax
	add rax, 8
	mov byte [rax], 0
	inc rax
	;Create link to previous word and 0-byte after it

	mov rsi, rax
	pop rdi
	push rax
	call string_copy
	pop rax
	push rax
	mov rdi, rax
	call string_length
	pop rdx
	add rdx, rax
	inc rdx
	;Create a name of word
	
	pop rax
	mov [rdx], al
	inc rdx
	;Create flags
	
	mov [here], rdx
	jmp next

;Read word from current input stream and start defining it
colon ":", colon
	.skip_blanks:
	dq xt_inbuf, xt_word	;read a word from stdin to input buffer
	branch0 .skip_blanks	;zero lenght means no letters in a word so we need to read another word
	dq xt_lit, 0			;push flags
	dq xt_inbuf, xt_create	;push word name and create a dictionary entry
	dq xt_lit, i_docol		;push a pointer at docol implementation
	dq xt_comma				;add docol impl as an impl of a new word
	dq xt_lit, 1, xt_state	;
	dq xt_store, xt_exit	;change state to compilation and exit the word "colon" definition

;End the current word definition
colon ";", semicolon, 1
	dq xt_lit, xt_exit, xt_comma	;append the "exit" word to the new word definition to complete it
	dq xt_lit, 0, xt_state			; 
	dq xt_store, xt_exit			;change state to interpretation an exit the word "semicolon"

section .data
main_stub:
 	dq xt_lit, 0, xt_interpret_fd, xt_bye


	

;Read word, find its XT, place on stack (or zero if no such word)
colon "'", tick, 1
	dq xt_inbuf, xt_word, xt_drop	;read the word to input buffer and drop it's length
	dq xt_inbuf, xt_find, xt_dup	;find the word in dictionary and duplicate it's address
	branch0 .word_found				;check if word not found
	dq xt_cfa, xt_exit				;found and push xt by the address of the word start
	.word_found:
	dq xt_lit, 0, xt_exit			;push 0 to indicate that word not found
	
;Forthress interpreter/compiler.
colon "interpret", interpret
	dq xt_dup, xt_find, xt_dup		;find word in dictionary by it's name
	branch0 .number					;if not found check if it is a number

	dq xt_cfa						;push xt instead of start address
	dq xt_state, xt_fetch			;get state
	branch0 .interpretation			;check if state is interpretation

	dq xt_dup, xt_lit, 1, xt_sub	;calculete flag address
	dq xt_fetch_b, xt_not			;get flag byte and invert it
	branch0 .interpretation			;0 means immedete word which needs to be interpreted

									;Here compilation starts	
	dq xt_swap, xt_drop				;drop inbuf addr
	dq xt_comma, xt_exit			;add xt to the word definition and exit

	.interpretation:
	dq xt_swap, xt_drop				;drop inbuf addr
	dq xt_execute, xt_exit			;execute word and exit

	.number:
	dq xt_drop, xt_dup				;drop dictionary addr which is 0 and dup inbuf addr for error msg
	dq xt_number, xt_swap			;trying to parse a number
	branch0 .not_a_word				;branch in case of fail

	dq xt_swap, xt_drop				;drop copy of inbuf addr
	dq xt_state, xt_fetch			;check the state
	branch0 .exit					;if state is interpretation than the work is done

	dq xt_here, xt_lit, 8
	dq xt_sub, xt_fetch, xt_dup	;get the privious xt to check the privious word
	dq xt_lit, xt_branch0			;get xt of branch0
	dq xt_equals, xt_not			;compare it to prev xt
	branch0 .prev_is_branch			;branch if matches

	dq xt_dup, xt_lit, xt_branch	;get xt of branch
	dq xt_equals, xt_not			;compare it to prev xt
	branch0 .prev_is_branch			;branch if matches
	
	dq xt_lit, xt_lit, xt_comma		;place lit before the number to push it when word will be executed

	.prev_is_branch:
	dq xt_drop, xt_comma, xt_exit	;place the number itself and exit

	.not_a_word:
	dq xt_drop, xt_prints			;print the input
	dq xt_lit, msg_not_a_word
	dq xt_prints					;print not a word message

	.exit:
	dq xt_exit

;Interpret everything read from file descriptor fd
;( fd -- )
colon "interpret-fd", interpret_fd
	dq xt_lit, in_fd
	dq xt_fetch, xt_to_r		;save previous fd in rstack
	dq xt_lit, in_fd, xt_store	;change fd to given value
	
	.loop:
	dq xt_inbuf, xt_word	;read next word
	branch0 .end			;if no word, end the loop

	dq xt_inbuf, xt_interpret	;interpret given word
	branch .loop			;repeat

	.end:
	dq xt_from_r, xt_lit
	dq in_fd, xt_store		;restore previos in_fd
	dq xt_exit



















