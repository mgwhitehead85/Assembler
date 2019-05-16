mov cx 10
fun [20] 2 0 100
mov ax [7]
put
halt








mov bx [19]
mov cx [bx+1]
mov [60] cx
mov cx [bx+2]
mov [61] cx
mov cx 0
get
cmp ax [60]
jb [51]
cmp ax [61]
ja [51]
put
add cx 1
add dx ax
cmp cx 10
jb [51]
mov ax dx
brk
jmp [32]
halt