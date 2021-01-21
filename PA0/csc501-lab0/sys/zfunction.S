.section .text
.global zfunction
zfunction:
push %ebp
movl %esp, %ebp
movl $0, %ebx
movl 8(%ebp), %ebx
andl $0xffc01fff, %ebx
shll $4, %ebx
andl $0xfffffff0, %ebx
movl %ebx, %eax
movl %ebp, %esp
pop %ebp
ret
