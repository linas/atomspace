	.file	"insane-O2.cc"
	.text
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC0:
	.string	"duuude enter FloatValue::operator=="
	.align 8
.LC1:
	.string	"duuude %zu lili=%lx and ulps = %d\n"
	.align 8
.LC2:
	.string	"duuude %zu lulu=%lx and ulps = %d and %d\n"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC3:
	.string	"duuude %zu lili is neg %d\n"
.LC4:
	.string	"duuude %zu lulu is neg %d\n"
	.section	.rodata.str1.8
	.align 8
.LC5:
	.string	"duuude %zu keep going because toobig = %d and %d\n"
	.section	.rodata.str1.1
.LC6:
	.string	"keep going"
	.text
	.p2align 4
	.globl	_Z7equal_pRKSt6vectorIdSaIdEES3_
	.type	_Z7equal_pRKSt6vectorIdSaIdEES3_, @function
_Z7equal_pRKSt6vectorIdSaIdEES3_:
.LFB1053:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	movq	%rsi, %r13
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movq	%rdi, %r12
	leaq	.LC0(%rip), %rdi
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$8, %rsp
	.cfi_def_cfa_offset 64
	call	puts@PLT
	movq	(%r12), %rax
	movq	8(%r12), %rdi
	movq	0(%r13), %rsi
	movq	8(%r13), %rbp
	subq	%rax, %rdi
	subq	%rsi, %rbp
	cmpq	%rdi, %rbp
	jne	.L2
	testq	%rbp, %rbp
	je	.L3
	sarq	$3, %rbp
	xorl	%ebx, %ebx
	jmp	.L4
	.p2align 4,,10
	.p2align 3
.L11:
	movq	%rbx, %rsi
	movl	%r15d, %ecx
	movl	%r15d, %edx
	xorl	%eax, %eax
	leaq	.LC5(%rip), %rdi
	addq	$1, %rbx
	call	printf@PLT
	leaq	.LC6(%rip), %rdi
	call	puts@PLT
	cmpq	%rbp, %rbx
	jnb	.L3
	movq	(%r12), %rax
	movq	0(%r13), %rsi
.L4:
	movq	(%rax,%rbx,8), %rax
	subq	(%rsi,%rbx,8), %rax
	leaq	.LC1(%rip), %rdi
	movq	%rbx, %rsi
	movq	%rax, %r14
	negq	%r14
	cmovs	%rax, %r14
	xorl	%ecx, %ecx
	cmpq	$24, %r14
	movq	%r14, %rdx
	setg	%cl
	xorl	%eax, %eax
	xorl	%r15d, %r15d
	call	printf@PLT
	cmpq	$24, %r14
	movq	%r14, %rdx
	movq	%rbx, %rsi
	seta	%r15b
	leaq	.LC2(%rip), %rdi
	xorl	%eax, %eax
	movl	%r15d, %r8d
	movl	%r15d, %ecx
	call	printf@PLT
	xorl	%edx, %edx
	movq	%rbx, %rsi
	xorl	%eax, %eax
	leaq	.LC3(%rip), %rdi
	call	printf@PLT
	xorl	%edx, %edx
	xorl	%eax, %eax
	movq	%rbx, %rsi
	leaq	.LC4(%rip), %rdi
	call	printf@PLT
	cmpq	$24, %r14
	jle	.L11
.L2:
	addq	$8, %rsp
	.cfi_remember_state
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
	.p2align 4,,10
	.p2align 3
.L3:
	.cfi_restore_state
	addq	$8, %rsp
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
	.cfi_endproc
.LFE1053:
	.size	_Z7equal_pRKSt6vectorIdSaIdEES3_, .-_Z7equal_pRKSt6vectorIdSaIdEES3_
	.section	.text._ZNSt12_Vector_baseIdSaIdEED2Ev,"axG",@progbits,_ZNSt12_Vector_baseIdSaIdEED5Ev,comdat
	.align 2
	.p2align 4
	.weak	_ZNSt12_Vector_baseIdSaIdEED2Ev
	.type	_ZNSt12_Vector_baseIdSaIdEED2Ev, @function
_ZNSt12_Vector_baseIdSaIdEED2Ev:
.LFB1086:
	.cfi_startproc
	movq	(%rdi), %rax
	testq	%rax, %rax
	je	.L12
	movq	16(%rdi), %rsi
	movq	%rax, %rdi
	subq	%rax, %rsi
	jmp	_ZdlPvm@PLT
	.p2align 4,,10
	.p2align 3
.L12:
	ret
	.cfi_endproc
.LFE1086:
	.size	_ZNSt12_Vector_baseIdSaIdEED2Ev, .-_ZNSt12_Vector_baseIdSaIdEED2Ev
	.weak	_ZNSt12_Vector_baseIdSaIdEED1Ev
	.set	_ZNSt12_Vector_baseIdSaIdEED1Ev,_ZNSt12_Vector_baseIdSaIdEED2Ev
	.section	.rodata._ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_.str1.8,"aMS",@progbits,1
	.align 8
.LC7:
	.string	"cannot create std::vector larger than max_size()"
	.section	.text._ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_,"axG",@progbits,_ZNSt6vectorIdSaIdEEC5ESt16initializer_listIdERKS0_,comdat
	.align 2
	.p2align 4
	.weak	_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_
	.type	_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_, @function
_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_:
.LFB1068:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA1068
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
	jb	.L22
	testq	%rbp, %rbp
	je	.L23
	movq	%rbp, %rdi
	movq	%rsi, %r12
.LEHB0:
	call	_Znwm@PLT
	leaq	(%rax,%rbp), %r13
	movq	%rax, (%rbx)
	movq	%rbp, %rdx
	movq	%r12, %rsi
	movq	%r13, 16(%rbx)
	movq	%rax, %rdi
	call	memcpy@PLT
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
.L23:
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
.L22:
	.cfi_restore_state
	leaq	.LC7(%rip), %rdi
	call	_ZSt20__throw_length_errorPKc@PLT
.LEHE0:
.L19:
	movq	%rax, %rbp
.L18:
	movq	%rbx, %rdi
	call	_ZNSt12_Vector_baseIdSaIdEED2Ev
	movq	%rbp, %rdi
.LEHB1:
	call	_Unwind_Resume@PLT
.LEHE1:
	.cfi_endproc
.LFE1068:
	.globl	__gxx_personality_v0
	.section	.gcc_except_table._ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_,"aG",@progbits,_ZNSt6vectorIdSaIdEEC5ESt16initializer_listIdERKS0_,comdat
.LLSDA1068:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1068-.LLSDACSB1068
.LLSDACSB1068:
	.uleb128 .LEHB0-.LFB1068
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L19-.LFB1068
	.uleb128 0
	.uleb128 .LEHB1-.LFB1068
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSE1068:
	.section	.text._ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_,"axG",@progbits,_ZNSt6vectorIdSaIdEEC5ESt16initializer_listIdERKS0_,comdat
	.size	_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_, .-_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_
	.weak	_ZNSt6vectorIdSaIdEEC1ESt16initializer_listIdERKS0_
	.set	_ZNSt6vectorIdSaIdEEC1ESt16initializer_listIdERKS0_,_ZNSt6vectorIdSaIdEEC2ESt16initializer_listIdERKS0_
	.section	.rodata.str1.1
.LC11:
	.string	"The end! Are they equaL? %d\n"
	.section	.text.unlikely,"ax",@progbits
.LCOLDB12:
	.section	.text.startup,"ax",@progbits
.LHOTB12:
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB1054:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA1054
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
	subq	$112, %rsp
	.cfi_def_cfa_offset 144
	movapd	.LC8(%rip), %xmm0
	movapd	.LC9(%rip), %xmm1
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
	movapd	.LC8(%rip), %xmm0
	movl	$4, %edx
	movaps	%xmm0, 80(%rsp)
	movapd	.LC10(%rip), %xmm0
	movaps	%xmm0, 96(%rsp)
.LEHB3:
	call	_ZNSt6vectorIdSaIdEEC1ESt16initializer_listIdERKS0_
.LEHE3:
	movq	%rbx, %rsi
	movq	%rbp, %rdi
.LEHB4:
	call	_Z7equal_pRKSt6vectorIdSaIdEES3_
	movzbl	%al, %esi
	leaq	.LC11(%rip), %rdi
	xorl	%eax, %eax
	call	printf@PLT
.LEHE4:
	movq	%rbx, %rdi
	call	_ZNSt12_Vector_baseIdSaIdEED2Ev
	movq	%rbp, %rdi
	call	_ZNSt12_Vector_baseIdSaIdEED2Ev
	addq	$112, %rsp
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
.L27:
	.cfi_restore_state
	movq	%rax, %r12
	jmp	.L26
.L28:
	movq	%rax, %r12
	jmp	.L25
	.section	.gcc_except_table,"a",@progbits
.LLSDA1054:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1054-.LLSDACSB1054
.LLSDACSB1054:
	.uleb128 .LEHB2-.LFB1054
	.uleb128 .LEHE2-.LEHB2
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB3-.LFB1054
	.uleb128 .LEHE3-.LEHB3
	.uleb128 .L27-.LFB1054
	.uleb128 0
	.uleb128 .LEHB4-.LFB1054
	.uleb128 .LEHE4-.LEHB4
	.uleb128 .L28-.LFB1054
	.uleb128 0
.LLSDACSE1054:
	.section	.text.startup
	.cfi_endproc
	.section	.text.unlikely
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDAC1054
	.type	main.cold, @function
main.cold:
.LFSB1054:
.L25:
	.cfi_def_cfa_offset 144
	.cfi_offset 3, -32
	.cfi_offset 6, -24
	.cfi_offset 12, -16
	movq	%rbx, %rdi
	call	_ZNSt12_Vector_baseIdSaIdEED2Ev
.L26:
	movq	%rbp, %rdi
	call	_ZNSt12_Vector_baseIdSaIdEED2Ev
	movq	%r12, %rdi
.LEHB5:
	call	_Unwind_Resume@PLT
.LEHE5:
	.cfi_endproc
.LFE1054:
	.section	.gcc_except_table
.LLSDAC1054:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSEC1054-.LLSDACSBC1054
.LLSDACSBC1054:
	.uleb128 .LEHB5-.LCOLDB12
	.uleb128 .LEHE5-.LEHB5
	.uleb128 0
	.uleb128 0
.LLSDACSEC1054:
	.section	.text.unlikely
	.section	.text.startup
	.size	main, .-main
	.section	.text.unlikely
	.size	main.cold, .-main.cold
.LCOLDE12:
	.section	.text.startup
.LHOTE12:
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC8:
	.long	0
	.long	0
	.long	0
	.long	1076232192
	.align 16
.LC9:
	.long	0
	.long	1077280768
	.long	0
	.long	1077968896
	.align 16
.LC10:
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
	.ident	"GCC: (Debian 12.2.0-14+deb12u1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
