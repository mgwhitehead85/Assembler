mov bx 0
get
add dx ax
add bx 1
cmp bx 10
je [12]
jmp [2]
mov [50] dx
mov ax [50]
put
halt