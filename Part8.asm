mov cx 4
fun [30] 2 10 144
fun [59] 2 10 144
fun [88] 2 10 144
halt









mov bx [29]
mov dx [bx+1]
mov bx [bx+2]
mov cx 0
get
mov [bx] ax
add cx 1
add bx 1
cmp cx dx
jb [48]
brk
jmp [38]
halt








mov bx [58]
mov dx [bx+1]
mov bx [bx+2]
mov cx 0
mov ax [bx]
put
add cx 1
add bx 1
cmp cx dx
jb [77]
brk
jmp [67]
halt








mov bx [87]
mov cx [bx+1]
mov [163] cx
add dx 1
mov cx 1
mov bx [bx+2]
mov ax [bx]
mov [164] ax
mov ax [bx+1]
cmp ax [164]
ja [112]
mov [bx] ax
mov ax [164]
add bx 1
mov [bx] ax
add cx 1
cmp cx [163]
jb [126]
cmp dx [163]
jb [128]
brk
jmp [100]
jmp [88]
halt