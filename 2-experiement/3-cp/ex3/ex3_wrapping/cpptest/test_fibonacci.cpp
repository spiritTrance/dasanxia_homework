int fibonacci(int a){
    return a == 1 ? a : a * fibonacci(a - 1);
}
int main(){
    return fibonacci(4);
}


//  注意文中的所有文段删掉都没啥影响，注意.globl一定要有main声明
// 	.file	"test.cpp"      //源文件
// 	.option nopic           // csapp讲过，忘了，跟重定位有关
// 	.text
// 	.align	1               // 对齐限制，单位为字节
// 	.globl	_Z9fibonaccii   // 不知道有什么用，删掉没影响，应该是标识这个标签的吧
// 	.type	_Z9fibonaccii, @function
// _Z9fibonaccii:
// .LFB0:
// 	.cfi_startproc
// 	addi	sp,sp,-32
// 	.cfi_def_cfa_offset 32
// 	sw	ra,28(sp)
// 	sw	s0,24(sp)
// 	.cfi_offset 1, -4
// 	.cfi_offset 8, -8
// 	addi	s0,sp,32
// 	.cfi_def_cfa 8, 0
// 	sw	a0,-20(s0)
// 	lw	a4,-20(s0)
// 	li	a5,1
// 	beq	a4,a5,.L2
// 	lw	a5,-20(s0)
// 	addi	a5,a5,-1
// 	mv	a0,a5
// 	call	_Z9fibonaccii
// 	mv	a4,a0
// 	lw	a5,-20(s0)
// 	mul	a5,a4,a5
// 	j	.L4
// .L2:
// 	lw	a5,-20(s0)
// .L4:
// 	mv	a0,a5
// 	lw	ra,28(sp)
// 	.cfi_restore 1
// 	lw	s0,24(sp)
// 	.cfi_restore 8
// 	.cfi_def_cfa 2, 32
// 	addi	sp,sp,32
// 	.cfi_def_cfa_offset 0
// 	jr	ra
// 	.cfi_endproc
// .LFE0:
// 	.size	_Z9fibonaccii, .-_Z9fibonaccii
// 	.align	1
// 	.globl	main
// 	.type	main, @function
// main:
// .LFB1:
// 	.cfi_startproc
// 	addi	sp,sp,-16
// 	.cfi_def_cfa_offset 16
// 	sw	ra,12(sp)
// 	sw	s0,8(sp)
// 	.cfi_offset 1, -4
// 	.cfi_offset 8, -8
// 	addi	s0,sp,16
// 	.cfi_def_cfa 8, 0
// 	li	a0,4
// 	call	_Z9fibonaccii
// 	mv	a5,a0
// 	nop
// 	mv	a0,a5
// 	lw	ra,12(sp)
// 	.cfi_restore 1
// 	lw	s0,8(sp)
// 	.cfi_restore 8
// 	.cfi_def_cfa 2, 16
// 	addi	sp,sp,16
// 	.cfi_def_cfa_offset 0
// 	jr	ra
// 	.cfi_endproc
// .LFE1:
// 	.size	main, .-main
// 	.ident	"GCC: (GNU) 9.2.0"
// 	.section	.note.GNU-stack,"",@progbits


// 最后改成这样还是能跑哈，那一切都好说了
// 	.globl	main
// _Z9fibonaccii:
// 	addi	sp,sp,-32
// 	sw	ra,28(sp)
// 	sw	s0,24(sp)
// 	addi	s0,sp,32
// 	sw	a0,-20(s0)
// 	lw	a4,-20(s0)
// 	li	a5,1
// 	beq	a4,a5,.L2
// 	lw	a5,-20(s0)
// 	addi	a5,a5,-1
// 	mv	a0,a5
// 	call	_Z9fibonaccii
// 	mv	a4,a0
// 	lw	a5,-20(s0)
// 	mul	a5,a4,a5
// 	j	.L4
// .L2:
// 	lw	a5,-20(s0)
// .L4:
// 	mv	a0,a5
// 	lw	ra,28(sp)
// 	lw	s0,24(sp)
// 	addi	sp,sp,32
// 	jr	ra
// main:
// 	addi	sp,sp,-16
// 	sw	ra,12(sp)
// 	sw	s0,8(sp)
// 	addi	s0,sp,16
// 	li	a0,4
// 	call	_Z9fibonaccii
// 	mv	a5,a0
// 	nop
// 	mv	a0,a5
// 	lw	ra,12(sp)
// 	lw	s0,8(sp)
// 	addi	sp,sp,16
// 	jr	ra
