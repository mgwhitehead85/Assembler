mov cx 4
fun [20] 2 [50] [51]
mov ax [7]
put
halt








mov bx [19]
mov bx [bx+1]
mov cx 0
get
cmp ax [bx]
jb [44]
cmp ax [bx+1]
ja [44]
put
add cx 1
add dx ax
cmp cx 10
jb [44]
mov ax dx
brk
jmp [26]
halt



0
100