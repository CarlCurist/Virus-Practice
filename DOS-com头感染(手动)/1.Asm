org 100h
call	Display
mov	di,NormalSize
mov	di,[di]
add	di,SelfSize
add	di,$$
mov	dx,di

mov	si,copystart
mov	cx,CopySize

copy:;把复制代码复制到文件最末尾
mov	al,[si]
mov	[di],al
inc	si
inc	di
dec	cx
jnz	copy

;一下设置复制代码中寄存器的数值，赋值完毕后跳转过去
mov	si,prg_end
mov	di,$$
mov	bx,NormalSize
mov	cx,[bx]
jmp	dx

copystart:;复制代码
mov	al,[si]
mov	[di],al
inc	si
inc	di
dec	cx
jnz	copystart
mov	ax,$$
jmp	ax
copyend:


Display:
 mov	ax, Message
 mov	bp, ax	
 mov	cx, 10		
 mov	ax, 1301h
mov	bx, 000ch 
	mov	dl, 0
	int	10h			; 
	ret
Message: db "i am virus"
NormalSize dw 0x0000
CopySize equ copyend-copystart
SelfSize equ $-$$
prg_end: