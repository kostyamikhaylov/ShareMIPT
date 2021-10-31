	in
	pop ax
	call factorial
	out
	hlt
factorial:
	push ax
	push 0
	jb error
	push ax
	push 0
	je known
	push ax
	push 1
	je known
	push 1
	pop dx
loop:
	push dx
	push ax
	mul
	pop dx
	push ax
	push 1
	sub
	pop ax
	push ax
	push 1
	jne loop
	pop bx
	push dx
	push bx
	ret
known:
	pop bx
	push 1
	push bx
	ret
error:
	push 666
	out
	hlt
