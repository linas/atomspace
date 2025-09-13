	.file	"insane-more-O2.cc"
	.text
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC0:
	.string	"duude lili=0x%lx lulu=0x%lx is it big? %d\n"
	.text
	.p2align 4
	.globl	_Z9is_it_bigl
	.type	_Z9is_it_bigl, @function
_Z9is_it_bigl:
.LFB1314:
	.cfi_startproc
	endbr64
	cmpq	$24, %rdi
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	%rdi, %rdx
	movq	%rdi, %rcx
	seta	%r8b
	seta	%bl
	movl	$2, %edi
	xorl	%eax, %eax
	movzbl	%r8b, %r8d
	leaq	.LC0(%rip), %rsi
	call	__printf_chk@PLT
	movl	%ebx, %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1314:
	.size	_Z9is_it_bigl, .-_Z9is_it_bigl
	.section	.rodata.str1.8
	.align 8
.LC1:
	.string	"duuude enter FloatValue::operator=="
	.align 8
.LC2:
	.string	"duuude %zu lili=0x%lx and toobig = %d\n"
	.align 8
.LC3:
	.string	"duuude %zu keep going because toobig = %d\n"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC4:
	.string	"keep going"
	.text
	.p2align 4
	.globl	_Z7equal_pRKSt6vectorIdSaIdEES3_
	.type	_Z7equal_pRKSt6vectorIdSaIdEES3_, @function
_Z7equal_pRKSt6vectorIdSaIdEES3_:
.LFB1315:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movq	%rdi, %r14
	leaq	.LC1(%rip), %rdi
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	movq	%rsi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$8, %rsp
	.cfi_def_cfa_offset 64
	call	puts@PLT
	movq	(%r14), %rax
	movq	8(%r14), %r8
	movq	0(%rbp), %rsi
	movq	8(%rbp), %rdi
	subq	%rax, %r8
	subq	%rsi, %rdi
	cmpq	%r8, %rdi
	jne	.L5
	movq	%rdi, %r15
	xorl	%r13d, %r13d
	sarq	$3, %r15
	testq	%rdi, %rdi
	jne	.L6
.L8:
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movl	$1, %eax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L13:
	.cfi_restore_state
	movq	%r13, %rdx
	xorl	%ecx, %ecx
	leaq	.LC3(%rip), %rsi
	xorl	%eax, %eax
	movl	$2, %edi
	addq	$1, %r13
	call	__printf_chk@PLT
	leaq	.LC4(%rip), %rdi
	call	puts@PLT
	cmpq	%r15, %r13
	jnb	.L8
	movq	(%r14), %rax
	movq	0(%rbp), %rsi
.L6:
	movq	(%rax,%r13,8), %rax
	subq	(%rsi,%r13,8), %rax
	movl	$2, %edi
	leaq	.LC0(%rip), %rsi
	movq	%rax, %rbx
	negq	%rbx
	cmovs	%rax, %rbx
	xorl	%r12d, %r12d
	cmpq	$24, %rbx
	movq	%rbx, %rcx
	movq	%rbx, %rdx
	seta	%r12b
	xorl	%eax, %eax
	movl	%r12d, %r8d
	call	__printf_chk@PLT
	xorl	%eax, %eax
	movl	%r12d, %r8d
	movq	%rbx, %rcx
	movq	%r13, %rdx
	leaq	.LC2(%rip), %rsi
	movl	$2, %edi
	call	__printf_chk@PLT
	cmpq	$24, %rbx
	jle	.L13
.L5:
	addq	$8, %rsp
	.cfi_def_cfa_offset 56
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1315:
	.size	_Z7equal_pRKSt6vectorIdSaIdEES3_, .-_Z7equal_pRKSt6vectorIdSaIdEES3_
	.section	.rodata._ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_.str1.8,"aMS",@progbits,1
	.align 8
.LC5:
	.string	"cannot create std::vector larger than max_size()"
	.section	.text._ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_,"axG",@progbits,_ZNSt6vectorIdSaIdEEC5ESt16initializer_listIdERKS0_,comdat
	.align 2
	.p2align 4
	.weak	_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_
	.type	_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_, @function
_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_:
.LFB1330:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA1330
	endbr64
	movabsq	$9223372036854775800, %rax
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	pxor	%xmm0, %xmm0
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	leaq	0(,%rdx,8), %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	movq	%rdi, %rbx
	subq	$8, %rsp
	.cfi_def_cfa_offset 48
	movq	$0, 16(%rdi)
	movups	%xmm0, (%rdi)
	cmpq	%rbp, %rax
	jb	.L28
	testq	%rbp, %rbp
	je	.L29
	movq	%rbp, %rdi
	movq	%rsi, %r12
.LEHB0:
	call	_Znwm@PLT
	leaq	(%rax,%rbp), %r13
	movq	%rax, (%rbx)
	movq	%rax, %rdi
	movq	%r13, 16(%rbx)
	cmpq	$8, %rbp
	je	.L18
	movq	%rbp, %rdx
	movq	%r12, %rsi
	call	memcpy@PLT
.L17:
	movq	%r13, 8(%rbx)
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L29:
	.cfi_restore_state
	xorl	%r13d, %r13d
	movq	$0, (%rdi)
	movq	$0, 16(%rdi)
	movq	%r13, 8(%rbx)
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L18:
	.cfi_restore_state
	movsd	(%r12), %xmm0
	movsd	%xmm0, (%rax)
	jmp	.L17
.L28:
	leaq	.LC5(%rip), %rdi
	call	_ZSt20__throw_length_errorPKc@PLT
.LEHE0:
.L22:
	endbr64
	movq	%rax, %rbp
.L20:
	movq	(%rbx), %rdi
	movq	16(%rbx), %rsi
	subq	%rdi, %rsi
	testq	%rdi, %rdi
	je	.L21
	call	_ZdlPvm@PLT
.L21:
	movq	%rbp, %rdi
.LEHB1:
	call	_Unwind_Resume@PLT
.LEHE1:
	.cfi_endproc
.LFE1330:
	.globl	__gxx_personality_v0
	.section	.gcc_except_table._ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_,"aG",@progbits,_ZNSt6vectorIdSaIdEEC5ESt16initializer_listIdERKS0_,comdat
.LLSDA1330:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1330-.LLSDACSB1330
.LLSDACSB1330:
	.uleb128 .LEHB0-.LFB1330
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L22-.LFB1330
	.uleb128 0
	.uleb128 .LEHB1-.LFB1330
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSE1330:
	.section	.text._ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_,"axG",@progbits,_ZNSt6vectorIdSaIdEEC5ESt16initializer_listIdERKS0_,comdat
	.size	_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_, .-_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_
	.weak	_ZNSt6vectorIdSaIdEEC1ESt16initializer_listIdERKS0_
	.set	_ZNSt6vectorIdSaIdEEC1ESt16initializer_listIdERKS0_,_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_
	.section	.text._ZNSt6vectorIdSaIdEED2Ev,"axG",@progbits,_ZNSt6vectorIdSaIdEED5Ev,comdat
	.align 2
	.p2align 4
	.weak	_ZNSt6vectorIdSaIdEED2Ev
	.type	_ZNSt6vectorIdSaIdEED2Ev, @function
_ZNSt6vectorIdSaIdEED2Ev:
.LFB1333:
	.cfi_startproc
	endbr64
	movq	(%rdi), %rax
	testq	%rax, %rax
	je	.L30
	movq	16(%rdi), %rsi
	movq	%rax, %rdi
	subq	%rax, %rsi
	jmp	_ZdlPvm@PLT
	.p2align 4,,10
	.p2align 3
.L30:
	ret
	.cfi_endproc
.LFE1333:
	.size	_ZNSt6vectorIdSaIdEED2Ev, .-_ZNSt6vectorIdSaIdEED2Ev
	.weak	_ZNSt6vectorIdSaIdEED1Ev
	.set	_ZNSt6vectorIdSaIdEED1Ev,_ZNSt6vectorIdSaIdEED2Ev
	.section	.rodata.str1.1
.LC9:
	.string	"The end! Are they equal? %d\n"
	.section	.text.unlikely,"ax",@progbits
.LCOLDB10:
	.section	.text.startup,"ax",@progbits
.LHOTB10:
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB1316:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA1316
	endbr64
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	movl	$4, %edx
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	addq	$-128, %rsp
	.cfi_def_cfa_offset 160
	movapd	.LC6(%rip), %xmm0
	movapd	.LC7(%rip), %xmm1
	movq	%fs:40, %rax
	movq	%rax, 120(%rsp)
	xorl	%eax, %eax
	leaq	80(%rsp), %r12
	leaq	48(%rsp), %rbx
	leaq	16(%rsp), %rbp
	movq	%rbx, %rcx
	movq	%r12, %rsi
	movaps	%xmm0, 80(%rsp)
	movq	%rbp, %rdi
	movaps	%xmm1, 96(%rsp)
.LEHB2:
	call	_ZNSt6vectorIdSaIdEEC1ESt16initializer_listIdERKS0_
.LEHE2:
	leaq	15(%rsp), %rcx
	movq	%r12, %rsi
	movq	%rbx, %rdi
	movapd	.LC6(%rip), %xmm0
	movl	$4, %edx
	movaps	%xmm0, 80(%rsp)
	movapd	.LC8(%rip), %xmm0
	movaps	%xmm0, 96(%rsp)
.LEHB3:
	call	_ZNSt6vectorIdSaIdEEC1ESt16initializer_listIdERKS0_
.LEHE3:
	movq	%rbx, %rsi
	movq	%rbp, %rdi
.LEHB4:
	call	_Z7equal_pRKSt6vectorIdSaIdEES3_
	movzbl	%al, %edx
	leaq	.LC9(%rip), %rsi
	movl	$2, %edi
	xorl	%eax, %eax
	call	__printf_chk@PLT
.LEHE4:
	movq	%rbx, %rdi
	call	_ZNSt6vectorIdSaIdEED1Ev
	movq	%rbp, %rdi
	call	_ZNSt6vectorIdSaIdEED1Ev
	movq	120(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L41
	subq	$-128, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 32
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
.L41:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
.L38:
	endbr64
	movq	%rax, %r12
	jmp	.L33
.L37:
	endbr64
	movq	%rax, %r12
	jmp	.L34
	.section	.gcc_except_table,"a",@progbits
.LLSDA1316:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1316-.LLSDACSB1316
.LLSDACSB1316:
	.uleb128 .LEHB2-.LFB1316
	.uleb128 .LEHE2-.LEHB2
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB3-.LFB1316
	.uleb128 .LEHE3-.LEHB3
	.uleb128 .L37-.LFB1316
	.uleb128 0
	.uleb128 .LEHB4-.LFB1316
	.uleb128 .LEHE4-.LEHB4
	.uleb128 .L38-.LFB1316
	.uleb128 0
.LLSDACSE1316:
	.section	.text.startup
	.cfi_endproc
	.section	.text.unlikely
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDAC1316
	.type	main.cold, @function
main.cold:
.LFSB1316:
.L33:
	.cfi_def_cfa_offset 160
	.cfi_offset 3, -32
	.cfi_offset 6, -24
	.cfi_offset 12, -16
	movq	%rbx, %rdi
	call	_ZNSt6vectorIdSaIdEED1Ev
.L34:
	movq	%rbp, %rdi
	call	_ZNSt6vectorIdSaIdEED1Ev
	movq	120(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L42
	movq	%r12, %rdi
.LEHB5:
	call	_Unwind_Resume@PLT
.LEHE5:
.L42:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1316:
	.section	.gcc_except_table
.LLSDAC1316:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSEC1316-.LLSDACSBC1316
.LLSDACSBC1316:
	.uleb128 .LEHB5-.LCOLDB10
	.uleb128 .LEHE5-.LEHB5
	.uleb128 0
	.uleb128 0
.LLSDACSEC1316:
	.section	.text.unlikely
	.section	.text.startup
	.size	main, .-main
	.section	.text.unlikely
	.size	main.cold, .-main.cold
.LCOLDE10:
	.section	.text.startup
.LHOTE10:
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC6:
	.long	0
	.long	0
	.long	0
	.long	1076232192
	.align 16
.LC7:
	.long	0
	.long	1077280768
	.long	0
	.long	1077968896
	.align 16
.LC8:
	.long	0
	.long	-1070202880
	.long	0
	.long	-1069514752
	.hidden	DW.ref.__gxx_personality_v0
	.weak	DW.ref.__gxx_personality_v0
	.section	.data.rel.local.DW.ref.__gxx_personality_v0,"awG",@progbits,DW.ref.__gxx_personality_v0,comdat
	.align 8
	.type	DW.ref.__gxx_personality_v0, @object
	.size	DW.ref.__gxx_personality_v0, 8
DW.ref.__gxx_personality_v0:
	.quad	__gxx_personality_v0
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
