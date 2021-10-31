	push 0
next:
	pop ax
	push ax
	push 10
	ja stop
	push ax
	push ax
	mul
	out
	push ax
	push 1
	add
	jmp next
stop:
	hlt
