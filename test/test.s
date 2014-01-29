	.file	"test.ll"
	.section	.debug_info,"",@progbits
.Lsection_info:
	.section	.debug_abbrev,"",@progbits
.Lsection_abbrev:
	.section	.debug_aranges,"",@progbits
	.section	.debug_macinfo,"",@progbits
	.section	.debug_line,"",@progbits
.Lsection_line:
	.section	.debug_loc,"",@progbits
	.section	.debug_pubnames,"",@progbits
	.section	.debug_pubtypes,"",@progbits
	.section	.debug_str,"MS",@progbits,1
.Linfo_string:
	.section	.debug_ranges,"",@progbits
.Ldebug_range:
	.section	.debug_loc,"",@progbits
.Lsection_debug_loc:
	.text
.Ltext_begin:
	.data
	.file	1 "test.wl"
	.file	2 "sdl.wl"
	.file	3 "cstdlib.wl"
	.file	4 "cstdio.wl"
	.text
	.globl	pixelIsWhite
	.align	16, 0x90
	.type	pixelIsWhite,@function
pixelIsWhite:                           # @pixelIsWhite
	.cfi_startproc
.Lfunc_begin0:
	.loc	1 20 0                  # test.wl:20:0
# BB#0:                                 # %entry
	pushq	%rbp
.Ltmp2:
	.cfi_def_cfa_offset 16
.Ltmp3:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp4:
	.cfi_def_cfa_register %rbp
	movq	%rdi, -16(%rbp)
	movl	%esi, -24(%rbp)
	movl	%edx, -32(%rbp)
	.loc	1 22 1 prologue_end     # test.wl:22:1
.Ltmp5:
	movq	-16(%rbp), %rax
	movq	8(%rax), %rcx
	movq	32(%rax), %rsi
	movzbl	9(%rcx), %ecx
	imull	-24(%rbp), %ecx
	movzwl	24(%rax), %eax
	imull	%edx, %eax
	addl	%ecx, %eax
	cltq
	cmpb	$0, (%rsi,%rax)
	setne	-40(%rbp)
	.loc	1 23 1                  # test.wl:23:1
	setne	-1(%rbp)
	movb	-1(%rbp), %al
	popq	%rbp
	ret
.Ltmp6:
.Ltmp7:
	.size	pixelIsWhite, .Ltmp7-pixelIsWhite
.Lfunc_end0:
	.cfi_endproc

	.globl	setPixel
	.align	16, 0x90
	.type	setPixel,@function
setPixel:                               # @setPixel
	.cfi_startproc
.Lfunc_begin1:
	.loc	1 26 0                  # test.wl:26:0
# BB#0:                                 # %entry
	pushq	%rbp
.Ltmp10:
	.cfi_def_cfa_offset 16
.Ltmp11:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp12:
	.cfi_def_cfa_register %rbp
	movq	%rdi, -8(%rbp)
	movl	%esi, -16(%rbp)
	movl	%edx, -24(%rbp)
	movl	%ecx, -32(%rbp)
	.loc	1 28 1 prologue_end     # test.wl:28:1
.Ltmp13:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rcx
	movq	32(%rax), %rdx
	movzbl	9(%rcx), %ecx
	imull	-16(%rbp), %ecx
	movzwl	24(%rax), %eax
	imull	-24(%rbp), %eax
	addl	%ecx, %eax
	cltq
	movb	-32(%rbp), %cl
	movb	%cl, (%rdx,%rax)
	popq	%rbp
	ret
.Ltmp14:
.Ltmp15:
	.size	setPixel, .Ltmp15-setPixel
.Lfunc_end1:
	.cfi_endproc

	.globl	update
	.align	16, 0x90
	.type	update,@function
update:                                 # @update
	.cfi_startproc
.Lfunc_begin2:
	.loc	1 31 0                  # test.wl:31:0
# BB#0:                                 # %entry
	pushq	%rbp
.Ltmp19:
	.cfi_def_cfa_offset 16
.Ltmp20:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp21:
	.cfi_def_cfa_register %rbp
	.loc	1 33 1 prologue_end     # test.wl:33:1
.Ltmp22:
	pushq	%rbx
	subq	$24, %rsp
.Ltmp23:
	.cfi_offset %rbx, -24
	movl	$0, -16(%rbp)
	.loc	1 34 1                  # test.wl:34:1
	movl	$1, -24(%rbp)
	jmp	.LBB2_1
	.align	16, 0x90
.LBB2_28:                               # %forupdate
                                        #   in Loop: Header=BB2_1 Depth=1
	incl	-24(%rbp)
.LBB2_1:                                # %for_condition
                                        # =>This Loop Header: Depth=1
                                        #     Child Loop BB2_3 Depth 2
	cmpl	$239, -24(%rbp)
	jg	.LBB2_27
# BB#2:                                 # %for_true
                                        #   in Loop: Header=BB2_1 Depth=1
	.loc	1 36 1                  # test.wl:36:1
	movq	%rsp, %rax
	leaq	-16(%rax), %rbx
	movq	%rbx, %rsp
	movl	$1, -16(%rax)
	jmp	.LBB2_3
	.align	16, 0x90
.LBB2_26:                               # %forupdate4
                                        #   in Loop: Header=BB2_3 Depth=2
	incl	(%rbx)
.LBB2_3:                                # %for_condition1
                                        #   Parent Loop BB2_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	cmpl	$319, (%rbx)            # imm = 0x13F
	jg	.LBB2_28
# BB#4:                                 # %for_true2
                                        #   in Loop: Header=BB2_3 Depth=2
	.loc	1 38 1                  # test.wl:38:1
	movl	$0, -16(%rbp)
	.loc	1 39 1                  # test.wl:39:1
	movq	surf(%rip), %rdi
	movl	(%rbx), %esi
	decl	%esi
	movl	-24(%rbp), %edx
	decl	%edx
	callq	pixelIsWhite
	testb	$1, %al
	je	.LBB2_6
# BB#5:                                 # %true
                                        #   in Loop: Header=BB2_3 Depth=2
	incl	-16(%rbp)
.LBB2_6:                                # %endif
                                        #   in Loop: Header=BB2_3 Depth=2
	.loc	1 40 1                  # test.wl:40:1
	movq	surf(%rip), %rdi
	movl	(%rbx), %esi
	movl	-24(%rbp), %edx
	decl	%edx
	callq	pixelIsWhite
	testb	$1, %al
	je	.LBB2_8
# BB#7:                                 # %true6
                                        #   in Loop: Header=BB2_3 Depth=2
	incl	-16(%rbp)
.LBB2_8:                                # %endif8
                                        #   in Loop: Header=BB2_3 Depth=2
	.loc	1 41 1                  # test.wl:41:1
	movq	surf(%rip), %rdi
	movl	(%rbx), %esi
	incl	%esi
	movl	-24(%rbp), %edx
	decl	%edx
	callq	pixelIsWhite
	testb	$1, %al
	je	.LBB2_10
# BB#9:                                 # %true9
                                        #   in Loop: Header=BB2_3 Depth=2
	incl	-16(%rbp)
.LBB2_10:                               # %endif11
                                        #   in Loop: Header=BB2_3 Depth=2
	.loc	1 42 1                  # test.wl:42:1
	movq	surf(%rip), %rdi
	movl	(%rbx), %esi
	decl	%esi
	movl	-24(%rbp), %edx
	callq	pixelIsWhite
	testb	$1, %al
	je	.LBB2_12
# BB#11:                                # %true12
                                        #   in Loop: Header=BB2_3 Depth=2
	incl	-16(%rbp)
.LBB2_12:                               # %endif14
                                        #   in Loop: Header=BB2_3 Depth=2
	.loc	1 43 1                  # test.wl:43:1
	movq	surf(%rip), %rdi
	movl	(%rbx), %esi
	incl	%esi
	movl	-24(%rbp), %edx
	callq	pixelIsWhite
	testb	$1, %al
	je	.LBB2_14
# BB#13:                                # %true15
                                        #   in Loop: Header=BB2_3 Depth=2
	incl	-16(%rbp)
.LBB2_14:                               # %endif17
                                        #   in Loop: Header=BB2_3 Depth=2
	.loc	1 44 1                  # test.wl:44:1
	movq	surf(%rip), %rdi
	movl	(%rbx), %esi
	decl	%esi
	movl	-24(%rbp), %edx
	incl	%edx
	callq	pixelIsWhite
	testb	$1, %al
	je	.LBB2_16
# BB#15:                                # %true18
                                        #   in Loop: Header=BB2_3 Depth=2
	incl	-16(%rbp)
.LBB2_16:                               # %endif20
                                        #   in Loop: Header=BB2_3 Depth=2
	.loc	1 45 1                  # test.wl:45:1
	movq	surf(%rip), %rdi
	movl	(%rbx), %esi
	movl	-24(%rbp), %edx
	incl	%edx
	callq	pixelIsWhite
	testb	$1, %al
	je	.LBB2_18
# BB#17:                                # %true21
                                        #   in Loop: Header=BB2_3 Depth=2
	incl	-16(%rbp)
.LBB2_18:                               # %endif23
                                        #   in Loop: Header=BB2_3 Depth=2
	.loc	1 46 1                  # test.wl:46:1
	movq	surf(%rip), %rdi
	movl	(%rbx), %esi
	incl	%esi
	movl	-24(%rbp), %edx
	incl	%edx
	callq	pixelIsWhite
	testb	$1, %al
	je	.LBB2_20
# BB#19:                                # %true24
                                        #   in Loop: Header=BB2_3 Depth=2
	incl	-16(%rbp)
.LBB2_20:                               # %endif26
                                        #   in Loop: Header=BB2_3 Depth=2
	.loc	1 48 1                  # test.wl:48:1
	cmpl	$1, -16(%rbp)
	jg	.LBB2_22
# BB#21:                                # %true27
                                        #   in Loop: Header=BB2_3 Depth=2
	movq	back(%rip), %rdi
	movl	(%rbx), %esi
	movl	-24(%rbp), %edx
	xorl	%ecx, %ecx
	callq	setPixel
.LBB2_22:                               # %endif29
                                        #   in Loop: Header=BB2_3 Depth=2
	.loc	1 49 1                  # test.wl:49:1
	cmpl	$3, -16(%rbp)
	jne	.LBB2_24
# BB#23:                                # %true30
                                        #   in Loop: Header=BB2_3 Depth=2
	movq	back(%rip), %rdi
	movl	(%rbx), %esi
	movl	-24(%rbp), %edx
	movl	$255, %ecx
	callq	setPixel
.LBB2_24:                               # %endif32
                                        #   in Loop: Header=BB2_3 Depth=2
	.loc	1 50 1                  # test.wl:50:1
	cmpl	$4, -16(%rbp)
	jl	.LBB2_26
# BB#25:                                # %true33
                                        #   in Loop: Header=BB2_3 Depth=2
	movq	back(%rip), %rdi
	movl	(%rbx), %esi
	movl	-24(%rbp), %edx
	xorl	%ecx, %ecx
	callq	setPixel
	jmp	.LBB2_26
.LBB2_27:                               # %exit
	.loc	1 53 1                  # test.wl:53:1
	movq	surf(%rip), %rax
	movq	32(%rax), %rdi
	movq	back(%rip), %rcx
	movq	32(%rcx), %rsi
	movzwl	24(%rax), %eax
	imull	$240, %eax, %edx
	callq	memcpy
	leaq	-8(%rbp), %rsp
	popq	%rbx
	popq	%rbp
	ret
.Ltmp24:
.Ltmp25:
	.size	update, .Ltmp25-update
.Lfunc_end2:
	.cfi_endproc

	.globl	randomize
	.align	16, 0x90
	.type	randomize,@function
randomize:                              # @randomize
	.cfi_startproc
.Lfunc_begin3:
	.loc	1 56 0                  # test.wl:56:0
# BB#0:                                 # %entry
	pushq	%rbp
.Ltmp29:
	.cfi_def_cfa_offset 16
.Ltmp30:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp31:
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	subq	$16, %rsp
.Ltmp32:
	.cfi_offset %rbx, -48
.Ltmp33:
	.cfi_offset %r12, -40
.Ltmp34:
	.cfi_offset %r14, -32
.Ltmp35:
	.cfi_offset %r15, -24
	movq	%rdi, -40(%rbp)
	.loc	1 58 1 prologue_end     # test.wl:58:1
.Ltmp36:
	movl	$0, -48(%rbp)
	jmp	.LBB3_1
	.align	16, 0x90
.LBB3_6:                                # %forupdate
                                        #   in Loop: Header=BB3_1 Depth=1
	incl	-48(%rbp)
.LBB3_1:                                # %for_condition
                                        # =>This Loop Header: Depth=1
                                        #     Child Loop BB3_3 Depth 2
	cmpl	$239, -48(%rbp)
	jg	.LBB3_5
# BB#2:                                 # %for_true
                                        #   in Loop: Header=BB3_1 Depth=1
	.loc	1 60 1                  # test.wl:60:1
	movq	%rsp, %rax
	leaq	-16(%rax), %r12
	movq	%r12, %rsp
	movl	$0, -16(%rax)
	jmp	.LBB3_3
	.align	16, 0x90
.LBB3_4:                                # %forupdate5
                                        #   in Loop: Header=BB3_3 Depth=2
	.loc	1 62 1                  # test.wl:62:1
	movq	%rsp, %rax
	addq	$-16, %rax
	movq	%rax, %rsp
	.loc	1 63 1                  # test.wl:63:1
	movq	-40(%rbp), %rbx
	movl	(%r12), %r14d
	movl	-48(%rbp), %r15d
	callq	rand
	movq	%rbx, %rdi
	movl	%r14d, %esi
	movl	%r15d, %edx
	movl	%eax, %ecx
	callq	setPixel
	.loc	1 60 1                  # test.wl:60:1
	incl	(%r12)
.LBB3_3:                                # %for_condition2
                                        #   Parent Loop BB3_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	cmpl	$319, (%r12)            # imm = 0x13F
	jle	.LBB3_4
	jmp	.LBB3_6
.LBB3_5:                                # %exit
	.loc	1 66 1                  # test.wl:66:1
	leaq	-32(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	ret
.Ltmp37:
.Ltmp38:
	.size	randomize, .Ltmp38-randomize
.Lfunc_end3:
	.cfi_endproc

	.globl	somefunc
	.align	16, 0x90
	.type	somefunc,@function
somefunc:                               # @somefunc
	.cfi_startproc
.Lfunc_begin4:
	.loc	1 69 0                  # test.wl:69:0
# BB#0:                                 # %entry
	pushq	%rbp
.Ltmp41:
	.cfi_def_cfa_offset 16
.Ltmp42:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp43:
	.cfi_def_cfa_register %rbp
	.loc	1 72 1 prologue_end     # test.wl:72:1
.Ltmp44:
	movq	testval(%rip), %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	popq	%rbp
	ret
.Ltmp45:
.Ltmp46:
	.size	somefunc, .Ltmp46-somefunc
.Lfunc_end4:
	.cfi_endproc

	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
.Lfunc_begin5:
	.loc	1 75 0                  # test.wl:75:0
# BB#0:                                 # %entry
	pushq	%rbp
.Ltmp50:
	.cfi_def_cfa_offset 16
.Ltmp51:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp52:
	.cfi_def_cfa_register %rbp
	pushq	%rbx
	subq	$56, %rsp
.Ltmp53:
	.cfi_offset %rbx, -24
	movl	%edi, -16(%rbp)
	movq	%rsi, -24(%rbp)
	.loc	1 77 1 prologue_end     # test.wl:77:1
.Ltmp54:
	movl	SDL_SWSURFACE(%rip), %ecx
	movl	$320, %edi              # imm = 0x140
	movl	$240, %esi
	xorl	%edx, %edx
	callq	SDL_SetVideoMode
	movq	%rax, surf(%rip)
	.loc	1 78 1                  # test.wl:78:1
	movl	SDL_SWSURFACE(%rip), %edi
	movq	8(%rax), %rax
	movzbl	8(%rax), %ecx
	movl	20(%rax), %r8d
	movl	24(%rax), %r9d
	movl	28(%rax), %edx
	movl	32(%rax), %eax
	subq	$16, %rsp
	movl	%eax, 8(%rsp)
	movl	%edx, (%rsp)
	movl	$320, %esi              # imm = 0x140
	movl	$240, %edx
	callq	SDL_CreateRGBSurface
	addq	$16, %rsp
	movq	%rax, back(%rip)
	movl	$100, %edi
	.loc	1 81 1                  # test.wl:81:1
	callq	srand
	.loc	1 82 1                  # test.wl:82:1
	movq	back(%rip), %rdi
	callq	randomize
	.loc	1 83 1                  # test.wl:83:1
	movq	surf(%rip), %rdi
	callq	randomize
	.loc	1 84 1                  # test.wl:84:1
	movq	$.L__unnamed_1, -32(%rbp)
	movl	$.L__unnamed_1, %edi
	xorl	%esi, %esi
	.loc	1 85 1                  # test.wl:85:1
	callq	SDL_WM_SetCaption
	.loc	1 87 1                  # test.wl:87:1
	movl	$5, -40(%rbp)
	.loc	1 88 1                  # test.wl:88:1
	movl	$1084227584, -48(%rbp)  # imm = 0x40A00000
	.loc	1 91 1                  # test.wl:91:1
	movb	$0, -56(%rbp)
	jmp	.LBB5_1
	.align	16, 0x90
.LBB5_2:                                # %while_true
                                        #   in Loop: Header=BB5_1 Depth=1
	.loc	1 94 1                  # test.wl:94:1
	callq	SDL_PumpEvents
	.loc	1 95 1                  # test.wl:95:1
	movq	%rsp, %rbx
	leaq	-16(%rbx), %rax
	movq	%rax, %rsp
	xorl	%edi, %edi
	callq	SDL_GetKeyState
	movq	%rax, -16(%rbx)
	.loc	1 96 1                  # test.wl:96:1
	movslq	SDLK_SPACE(%rip), %rcx
	movb	(%rax,%rcx), %al
	movb	%al, -56(%rbp)
	movl	$32, %edi
	.loc	1 97 1                  # test.wl:97:1
	callq	SDL_Delay
	.loc	1 98 1                  # test.wl:98:1
	movq	surf(%rip), %rdi
	callq	SDL_Flip
	.loc	1 99 1                  # test.wl:99:1
	callq	update
.LBB5_1:                                # %while_condition
                                        # =>This Inner Loop Header: Depth=1
	.loc	1 92 1                  # test.wl:92:1
	cmpb	$0, -56(%rbp)
	je	.LBB5_2
# BB#3:                                 # %exit
	.loc	1 102 1                 # test.wl:102:1
	movl	$0, -12(%rbp)
	xorl	%eax, %eax
	leaq	-8(%rbp), %rsp
	popq	%rbx
	popq	%rbp
	ret
.Ltmp55:
.Ltmp56:
	.size	main, .Ltmp56-main
.Lfunc_end5:
	.cfi_endproc

	.globl	Mix_PlayChannel
	.align	16, 0x90
	.type	Mix_PlayChannel,@function
Mix_PlayChannel:                        # @Mix_PlayChannel
	.cfi_startproc
.Lfunc_begin6:
	.loc	2 245 0                 # sdl.wl:245:0
# BB#0:                                 # %entry
	pushq	%rbp
.Ltmp59:
	.cfi_def_cfa_offset 16
.Ltmp60:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp61:
	.cfi_def_cfa_register %rbp
	subq	$32, %rsp
	movl	%edi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	%edx, -24(%rbp)
	.loc	2 246 1 prologue_end    # sdl.wl:246:1
.Ltmp62:
	movl	-8(%rbp), %edi
	movq	-16(%rbp), %rsi
	movl	$-1, %ecx
	callq	Mix_PlayChannelTimed
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	addq	$32, %rsp
	popq	%rbp
	ret
.Ltmp63:
.Ltmp64:
	.size	Mix_PlayChannel, .Ltmp64-Mix_PlayChannel
.Lfunc_end6:
	.cfi_endproc

	.globl	assert
	.align	16, 0x90
	.type	assert,@function
assert:                                 # @assert
	.cfi_startproc
.Lfunc_begin7:
	.loc	3 56 0                  # cstdlib.wl:56:0
# BB#0:                                 # %entry
	pushq	%rbp
.Ltmp67:
	.cfi_def_cfa_offset 16
.Ltmp68:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp69:
	.cfi_def_cfa_register %rbp
	subq	$16, %rsp
	andl	$1, %edi
	movb	%dil, -8(%rbp)
	movq	%rsi, -16(%rbp)
	.loc	3 58 1 prologue_end     # cstdlib.wl:58:1
.Ltmp70:
	movb	-8(%rbp), %al
	testb	%al, %al
	je	.LBB7_2
# BB#1:                                 # %true
	movq	-16(%rbp), %rdi
	movq	stderr(%rip), %rsi
	callq	fputs
.LBB7_2:                                # %exit
	.loc	3 59 1                  # cstdlib.wl:59:1
	movq	stderr(%rip), %rdi
	callq	fflush
	.loc	3 60 1                  # cstdlib.wl:60:1
	callq	abort
	addq	$16, %rsp
	popq	%rbp
	ret
.Ltmp71:
.Ltmp72:
	.size	assert, .Ltmp72-assert
.Lfunc_end7:
	.cfi_endproc

	.type	RAND_MAX1,@object       # @RAND_MAX1
	.section	.data.RAND_MAX1,"aGw",@progbits,RAND_MAX1,comdat
	.weak	RAND_MAX1
	.align	4
RAND_MAX1:
	.long	2147483647              # 0x7fffffff
	.size	RAND_MAX1, 4

	.type	surf,@object            # @surf
	.section	.bss.surf,"aGw",@nobits,surf,comdat
	.weak	surf
	.align	8
surf:
	.quad	0
	.size	surf, 8

	.type	back,@object            # @back
	.section	.bss.back,"aGw",@nobits,back,comdat
	.weak	back
	.align	8
back:
	.quad	0
	.size	back, 8

	.type	testval,@object         # @testval
	.section	.bss.testval,"aGw",@nobits,testval,comdat
	.weak	testval
	.align	8
testval:
	.quad	0
	.size	testval, 8

	.type	.L__unnamed_1,@object   # @0
	.section	.rodata,"a",@progbits
.L__unnamed_1:
	.asciz	"Life"
	.size	.L__unnamed_1, 5

	.type	SDL_INIT_TIMER,@object  # @SDL_INIT_TIMER
	.section	.data.SDL_INIT_TIMER,"aGw",@progbits,SDL_INIT_TIMER,comdat
	.weak	SDL_INIT_TIMER
	.align	4
SDL_INIT_TIMER:
	.long	1                       # 0x1
	.size	SDL_INIT_TIMER, 4

	.type	SDL_INIT_AUDIO,@object  # @SDL_INIT_AUDIO
	.section	.data.SDL_INIT_AUDIO,"aGw",@progbits,SDL_INIT_AUDIO,comdat
	.weak	SDL_INIT_AUDIO
	.align	4
SDL_INIT_AUDIO:
	.long	16                      # 0x10
	.size	SDL_INIT_AUDIO, 4

	.type	SDL_INIT_VIDEO,@object  # @SDL_INIT_VIDEO
	.section	.data.SDL_INIT_VIDEO,"aGw",@progbits,SDL_INIT_VIDEO,comdat
	.weak	SDL_INIT_VIDEO
	.align	4
SDL_INIT_VIDEO:
	.long	32                      # 0x20
	.size	SDL_INIT_VIDEO, 4

	.type	SDL_INIT_CDROM,@object  # @SDL_INIT_CDROM
	.section	.data.SDL_INIT_CDROM,"aGw",@progbits,SDL_INIT_CDROM,comdat
	.weak	SDL_INIT_CDROM
	.align	4
SDL_INIT_CDROM:
	.long	256                     # 0x100
	.size	SDL_INIT_CDROM, 4

	.type	SDL_INIT_JOYSTICK,@object # @SDL_INIT_JOYSTICK
	.section	.data.SDL_INIT_JOYSTICK,"aGw",@progbits,SDL_INIT_JOYSTICK,comdat
	.weak	SDL_INIT_JOYSTICK
	.align	4
SDL_INIT_JOYSTICK:
	.long	512                     # 0x200
	.size	SDL_INIT_JOYSTICK, 4

	.type	SDL_INIT_EVERYTHING,@object # @SDL_INIT_EVERYTHING
	.section	.data.SDL_INIT_EVERYTHING,"aGw",@progbits,SDL_INIT_EVERYTHING,comdat
	.weak	SDL_INIT_EVERYTHING
	.align	4
SDL_INIT_EVERYTHING:
	.long	65535                   # 0xffff
	.size	SDL_INIT_EVERYTHING, 4

	.type	SDL_APPMOUSEFOCUS,@object # @SDL_APPMOUSEFOCUS
	.section	.data.SDL_APPMOUSEFOCUS,"aGw",@progbits,SDL_APPMOUSEFOCUS,comdat
	.weak	SDL_APPMOUSEFOCUS
	.align	4
SDL_APPMOUSEFOCUS:
	.long	1                       # 0x1
	.size	SDL_APPMOUSEFOCUS, 4

	.type	SDL_APPINPUTFOCUS,@object # @SDL_APPINPUTFOCUS
	.section	.data.SDL_APPINPUTFOCUS,"aGw",@progbits,SDL_APPINPUTFOCUS,comdat
	.weak	SDL_APPINPUTFOCUS
	.align	4
SDL_APPINPUTFOCUS:
	.long	2                       # 0x2
	.size	SDL_APPINPUTFOCUS, 4

	.type	SDL_APPACTIVE,@object   # @SDL_APPACTIVE
	.section	.data.SDL_APPACTIVE,"aGw",@progbits,SDL_APPACTIVE,comdat
	.weak	SDL_APPACTIVE
	.align	4
SDL_APPACTIVE:
	.long	4                       # 0x4
	.size	SDL_APPACTIVE, 4

	.type	AUDIO_U8,@object        # @AUDIO_U8
	.section	.data.AUDIO_U8,"aGw",@progbits,AUDIO_U8,comdat
	.weak	AUDIO_U8
	.align	4
AUDIO_U8:
	.long	8                       # 0x8
	.size	AUDIO_U8, 4

	.type	AUDIO_S8,@object        # @AUDIO_S8
	.section	.data.AUDIO_S8,"aGw",@progbits,AUDIO_S8,comdat
	.weak	AUDIO_S8
	.align	4
AUDIO_S8:
	.long	32776                   # 0x8008
	.size	AUDIO_S8, 4

	.type	AUDIO_U16LSB,@object    # @AUDIO_U16LSB
	.section	.data.AUDIO_U16LSB,"aGw",@progbits,AUDIO_U16LSB,comdat
	.weak	AUDIO_U16LSB
	.align	4
AUDIO_U16LSB:
	.long	16                      # 0x10
	.size	AUDIO_U16LSB, 4

	.type	AUDIO_S16LSB,@object    # @AUDIO_S16LSB
	.section	.data.AUDIO_S16LSB,"aGw",@progbits,AUDIO_S16LSB,comdat
	.weak	AUDIO_S16LSB
	.align	4
AUDIO_S16LSB:
	.long	32784                   # 0x8010
	.size	AUDIO_S16LSB, 4

	.type	SDL_AUDIO_STOPPED,@object # @SDL_AUDIO_STOPPED
	.section	.bss.SDL_AUDIO_STOPPED,"aGw",@nobits,SDL_AUDIO_STOPPED,comdat
	.weak	SDL_AUDIO_STOPPED
	.align	4
SDL_AUDIO_STOPPED:
	.long	0                       # 0x0
	.size	SDL_AUDIO_STOPPED, 4

	.type	SDL_AUDIO_PLAYING,@object # @SDL_AUDIO_PLAYING
	.section	.data.SDL_AUDIO_PLAYING,"aGw",@progbits,SDL_AUDIO_PLAYING,comdat
	.weak	SDL_AUDIO_PLAYING
	.align	4
SDL_AUDIO_PLAYING:
	.long	1                       # 0x1
	.size	SDL_AUDIO_PLAYING, 4

	.type	SDL_AUDIO_PAUSED,@object # @SDL_AUDIO_PAUSED
	.section	.data.SDL_AUDIO_PAUSED,"aGw",@progbits,SDL_AUDIO_PAUSED,comdat
	.weak	SDL_AUDIO_PAUSED
	.align	4
SDL_AUDIO_PAUSED:
	.long	2                       # 0x2
	.size	SDL_AUDIO_PAUSED, 4

	.type	IMG_INIT_JPG,@object    # @IMG_INIT_JPG
	.section	.data.IMG_INIT_JPG,"aGw",@progbits,IMG_INIT_JPG,comdat
	.weak	IMG_INIT_JPG
	.align	4
IMG_INIT_JPG:
	.long	1                       # 0x1
	.size	IMG_INIT_JPG, 4

	.type	IMG_INIT_PNG,@object    # @IMG_INIT_PNG
	.section	.data.IMG_INIT_PNG,"aGw",@progbits,IMG_INIT_PNG,comdat
	.weak	IMG_INIT_PNG
	.align	4
IMG_INIT_PNG:
	.long	2                       # 0x2
	.size	IMG_INIT_PNG, 4

	.type	IMG_INIT_TIF,@object    # @IMG_INIT_TIF
	.section	.data.IMG_INIT_TIF,"aGw",@progbits,IMG_INIT_TIF,comdat
	.weak	IMG_INIT_TIF
	.align	4
IMG_INIT_TIF:
	.long	4                       # 0x4
	.size	IMG_INIT_TIF, 4

	.type	IMG_INIT_WEBP,@object   # @IMG_INIT_WEBP
	.section	.data.IMG_INIT_WEBP,"aGw",@progbits,IMG_INIT_WEBP,comdat
	.weak	IMG_INIT_WEBP
	.align	4
IMG_INIT_WEBP:
	.long	8                       # 0x8
	.size	IMG_INIT_WEBP, 4

	.type	SDL_DEFAULT_REPEAT_DELAY,@object # @SDL_DEFAULT_REPEAT_DELAY
	.section	.data.SDL_DEFAULT_REPEAT_DELAY,"aGw",@progbits,SDL_DEFAULT_REPEAT_DELAY,comdat
	.weak	SDL_DEFAULT_REPEAT_DELAY
	.align	4
SDL_DEFAULT_REPEAT_DELAY:
	.long	500                     # 0x1f4
	.size	SDL_DEFAULT_REPEAT_DELAY, 4

	.type	SDL_DEFAULT_REPEAT_INTERVAL,@object # @SDL_DEFAULT_REPEAT_INTERVAL
	.section	.data.SDL_DEFAULT_REPEAT_INTERVAL,"aGw",@progbits,SDL_DEFAULT_REPEAT_INTERVAL,comdat
	.weak	SDL_DEFAULT_REPEAT_INTERVAL
	.align	4
SDL_DEFAULT_REPEAT_INTERVAL:
	.long	30                      # 0x1e
	.size	SDL_DEFAULT_REPEAT_INTERVAL, 4

	.type	SDLK_BACKSPACE,@object  # @SDLK_BACKSPACE
	.section	.data.SDLK_BACKSPACE,"aGw",@progbits,SDLK_BACKSPACE,comdat
	.weak	SDLK_BACKSPACE
	.align	4
SDLK_BACKSPACE:
	.long	8                       # 0x8
	.size	SDLK_BACKSPACE, 4

	.type	SDLK_TAB,@object        # @SDLK_TAB
	.section	.data.SDLK_TAB,"aGw",@progbits,SDLK_TAB,comdat
	.weak	SDLK_TAB
	.align	4
SDLK_TAB:
	.long	9                       # 0x9
	.size	SDLK_TAB, 4

	.type	SDLK_CLEAR,@object      # @SDLK_CLEAR
	.section	.data.SDLK_CLEAR,"aGw",@progbits,SDLK_CLEAR,comdat
	.weak	SDLK_CLEAR
	.align	4
SDLK_CLEAR:
	.long	12                      # 0xc
	.size	SDLK_CLEAR, 4

	.type	SDLK_RETURN,@object     # @SDLK_RETURN
	.section	.data.SDLK_RETURN,"aGw",@progbits,SDLK_RETURN,comdat
	.weak	SDLK_RETURN
	.align	4
SDLK_RETURN:
	.long	13                      # 0xd
	.size	SDLK_RETURN, 4

	.type	SDLK_PAUSE,@object      # @SDLK_PAUSE
	.section	.data.SDLK_PAUSE,"aGw",@progbits,SDLK_PAUSE,comdat
	.weak	SDLK_PAUSE
	.align	4
SDLK_PAUSE:
	.long	19                      # 0x13
	.size	SDLK_PAUSE, 4

	.type	SDLK_ESCAPE,@object     # @SDLK_ESCAPE
	.section	.data.SDLK_ESCAPE,"aGw",@progbits,SDLK_ESCAPE,comdat
	.weak	SDLK_ESCAPE
	.align	4
SDLK_ESCAPE:
	.long	27                      # 0x1b
	.size	SDLK_ESCAPE, 4

	.type	SDLK_SPACE,@object      # @SDLK_SPACE
	.section	.data.SDLK_SPACE,"aGw",@progbits,SDLK_SPACE,comdat
	.weak	SDLK_SPACE
	.align	4
SDLK_SPACE:
	.long	32                      # 0x20
	.size	SDLK_SPACE, 4

	.type	SDLK_EXCLAIM,@object    # @SDLK_EXCLAIM
	.section	.data.SDLK_EXCLAIM,"aGw",@progbits,SDLK_EXCLAIM,comdat
	.weak	SDLK_EXCLAIM
	.align	4
SDLK_EXCLAIM:
	.long	33                      # 0x21
	.size	SDLK_EXCLAIM, 4

	.type	SDLK_QUOTEDBL,@object   # @SDLK_QUOTEDBL
	.section	.data.SDLK_QUOTEDBL,"aGw",@progbits,SDLK_QUOTEDBL,comdat
	.weak	SDLK_QUOTEDBL
	.align	4
SDLK_QUOTEDBL:
	.long	34                      # 0x22
	.size	SDLK_QUOTEDBL, 4

	.type	SDLK_HASH,@object       # @SDLK_HASH
	.section	.data.SDLK_HASH,"aGw",@progbits,SDLK_HASH,comdat
	.weak	SDLK_HASH
	.align	4
SDLK_HASH:
	.long	35                      # 0x23
	.size	SDLK_HASH, 4

	.type	SDLK_DOLLAR,@object     # @SDLK_DOLLAR
	.section	.data.SDLK_DOLLAR,"aGw",@progbits,SDLK_DOLLAR,comdat
	.weak	SDLK_DOLLAR
	.align	4
SDLK_DOLLAR:
	.long	36                      # 0x24
	.size	SDLK_DOLLAR, 4

	.type	SDLK_AMPERSAND,@object  # @SDLK_AMPERSAND
	.section	.data.SDLK_AMPERSAND,"aGw",@progbits,SDLK_AMPERSAND,comdat
	.weak	SDLK_AMPERSAND
	.align	4
SDLK_AMPERSAND:
	.long	38                      # 0x26
	.size	SDLK_AMPERSAND, 4

	.type	SDLK_QUOTE,@object      # @SDLK_QUOTE
	.section	.data.SDLK_QUOTE,"aGw",@progbits,SDLK_QUOTE,comdat
	.weak	SDLK_QUOTE
	.align	4
SDLK_QUOTE:
	.long	39                      # 0x27
	.size	SDLK_QUOTE, 4

	.type	SDLK_LEFTPAREN,@object  # @SDLK_LEFTPAREN
	.section	.data.SDLK_LEFTPAREN,"aGw",@progbits,SDLK_LEFTPAREN,comdat
	.weak	SDLK_LEFTPAREN
	.align	4
SDLK_LEFTPAREN:
	.long	40                      # 0x28
	.size	SDLK_LEFTPAREN, 4

	.type	SDLK_RIGHTPAREN,@object # @SDLK_RIGHTPAREN
	.section	.data.SDLK_RIGHTPAREN,"aGw",@progbits,SDLK_RIGHTPAREN,comdat
	.weak	SDLK_RIGHTPAREN
	.align	4
SDLK_RIGHTPAREN:
	.long	41                      # 0x29
	.size	SDLK_RIGHTPAREN, 4

	.type	SDLK_ASTERISK,@object   # @SDLK_ASTERISK
	.section	.data.SDLK_ASTERISK,"aGw",@progbits,SDLK_ASTERISK,comdat
	.weak	SDLK_ASTERISK
	.align	4
SDLK_ASTERISK:
	.long	42                      # 0x2a
	.size	SDLK_ASTERISK, 4

	.type	SDLK_PLUS,@object       # @SDLK_PLUS
	.section	.data.SDLK_PLUS,"aGw",@progbits,SDLK_PLUS,comdat
	.weak	SDLK_PLUS
	.align	4
SDLK_PLUS:
	.long	43                      # 0x2b
	.size	SDLK_PLUS, 4

	.type	SDLK_COMMA,@object      # @SDLK_COMMA
	.section	.data.SDLK_COMMA,"aGw",@progbits,SDLK_COMMA,comdat
	.weak	SDLK_COMMA
	.align	4
SDLK_COMMA:
	.long	44                      # 0x2c
	.size	SDLK_COMMA, 4

	.type	SDLK_MINUS,@object      # @SDLK_MINUS
	.section	.data.SDLK_MINUS,"aGw",@progbits,SDLK_MINUS,comdat
	.weak	SDLK_MINUS
	.align	4
SDLK_MINUS:
	.long	45                      # 0x2d
	.size	SDLK_MINUS, 4

	.type	SDLK_PERIOD,@object     # @SDLK_PERIOD
	.section	.data.SDLK_PERIOD,"aGw",@progbits,SDLK_PERIOD,comdat
	.weak	SDLK_PERIOD
	.align	4
SDLK_PERIOD:
	.long	46                      # 0x2e
	.size	SDLK_PERIOD, 4

	.type	SDLK_SLASH,@object      # @SDLK_SLASH
	.section	.data.SDLK_SLASH,"aGw",@progbits,SDLK_SLASH,comdat
	.weak	SDLK_SLASH
	.align	4
SDLK_SLASH:
	.long	47                      # 0x2f
	.size	SDLK_SLASH, 4

	.type	SDLK_0,@object          # @SDLK_0
	.section	.data.SDLK_0,"aGw",@progbits,SDLK_0,comdat
	.weak	SDLK_0
	.align	4
SDLK_0:
	.long	48                      # 0x30
	.size	SDLK_0, 4

	.type	SDLK_1,@object          # @SDLK_1
	.section	.data.SDLK_1,"aGw",@progbits,SDLK_1,comdat
	.weak	SDLK_1
	.align	4
SDLK_1:
	.long	49                      # 0x31
	.size	SDLK_1, 4

	.type	SDLK_2,@object          # @SDLK_2
	.section	.data.SDLK_2,"aGw",@progbits,SDLK_2,comdat
	.weak	SDLK_2
	.align	4
SDLK_2:
	.long	50                      # 0x32
	.size	SDLK_2, 4

	.type	SDLK_3,@object          # @SDLK_3
	.section	.data.SDLK_3,"aGw",@progbits,SDLK_3,comdat
	.weak	SDLK_3
	.align	4
SDLK_3:
	.long	51                      # 0x33
	.size	SDLK_3, 4

	.type	SDLK_4,@object          # @SDLK_4
	.section	.data.SDLK_4,"aGw",@progbits,SDLK_4,comdat
	.weak	SDLK_4
	.align	4
SDLK_4:
	.long	52                      # 0x34
	.size	SDLK_4, 4

	.type	SDLK_5,@object          # @SDLK_5
	.section	.data.SDLK_5,"aGw",@progbits,SDLK_5,comdat
	.weak	SDLK_5
	.align	4
SDLK_5:
	.long	53                      # 0x35
	.size	SDLK_5, 4

	.type	SDLK_6,@object          # @SDLK_6
	.section	.data.SDLK_6,"aGw",@progbits,SDLK_6,comdat
	.weak	SDLK_6
	.align	4
SDLK_6:
	.long	54                      # 0x36
	.size	SDLK_6, 4

	.type	SDLK_7,@object          # @SDLK_7
	.section	.data.SDLK_7,"aGw",@progbits,SDLK_7,comdat
	.weak	SDLK_7
	.align	4
SDLK_7:
	.long	55                      # 0x37
	.size	SDLK_7, 4

	.type	SDLK_8,@object          # @SDLK_8
	.section	.data.SDLK_8,"aGw",@progbits,SDLK_8,comdat
	.weak	SDLK_8
	.align	4
SDLK_8:
	.long	56                      # 0x38
	.size	SDLK_8, 4

	.type	SDLK_9,@object          # @SDLK_9
	.section	.data.SDLK_9,"aGw",@progbits,SDLK_9,comdat
	.weak	SDLK_9
	.align	4
SDLK_9:
	.long	57                      # 0x39
	.size	SDLK_9, 4

	.type	SDLK_COLON,@object      # @SDLK_COLON
	.section	.data.SDLK_COLON,"aGw",@progbits,SDLK_COLON,comdat
	.weak	SDLK_COLON
	.align	4
SDLK_COLON:
	.long	58                      # 0x3a
	.size	SDLK_COLON, 4

	.type	SDLK_SEMICOLON,@object  # @SDLK_SEMICOLON
	.section	.data.SDLK_SEMICOLON,"aGw",@progbits,SDLK_SEMICOLON,comdat
	.weak	SDLK_SEMICOLON
	.align	4
SDLK_SEMICOLON:
	.long	59                      # 0x3b
	.size	SDLK_SEMICOLON, 4

	.type	SDLK_LESS,@object       # @SDLK_LESS
	.section	.data.SDLK_LESS,"aGw",@progbits,SDLK_LESS,comdat
	.weak	SDLK_LESS
	.align	4
SDLK_LESS:
	.long	60                      # 0x3c
	.size	SDLK_LESS, 4

	.type	SDLK_EQUALS,@object     # @SDLK_EQUALS
	.section	.data.SDLK_EQUALS,"aGw",@progbits,SDLK_EQUALS,comdat
	.weak	SDLK_EQUALS
	.align	4
SDLK_EQUALS:
	.long	61                      # 0x3d
	.size	SDLK_EQUALS, 4

	.type	SDLK_GREATER,@object    # @SDLK_GREATER
	.section	.data.SDLK_GREATER,"aGw",@progbits,SDLK_GREATER,comdat
	.weak	SDLK_GREATER
	.align	4
SDLK_GREATER:
	.long	62                      # 0x3e
	.size	SDLK_GREATER, 4

	.type	SDLK_QUESTION,@object   # @SDLK_QUESTION
	.section	.data.SDLK_QUESTION,"aGw",@progbits,SDLK_QUESTION,comdat
	.weak	SDLK_QUESTION
	.align	4
SDLK_QUESTION:
	.long	63                      # 0x3f
	.size	SDLK_QUESTION, 4

	.type	SDLK_AT,@object         # @SDLK_AT
	.section	.data.SDLK_AT,"aGw",@progbits,SDLK_AT,comdat
	.weak	SDLK_AT
	.align	4
SDLK_AT:
	.long	64                      # 0x40
	.size	SDLK_AT, 4

	.type	SDLK_LEFTBRACKET,@object # @SDLK_LEFTBRACKET
	.section	.data.SDLK_LEFTBRACKET,"aGw",@progbits,SDLK_LEFTBRACKET,comdat
	.weak	SDLK_LEFTBRACKET
	.align	4
SDLK_LEFTBRACKET:
	.long	91                      # 0x5b
	.size	SDLK_LEFTBRACKET, 4

	.type	SDLK_BACKSLASH,@object  # @SDLK_BACKSLASH
	.section	.data.SDLK_BACKSLASH,"aGw",@progbits,SDLK_BACKSLASH,comdat
	.weak	SDLK_BACKSLASH
	.align	4
SDLK_BACKSLASH:
	.long	92                      # 0x5c
	.size	SDLK_BACKSLASH, 4

	.type	SDLK_RIGHTBRACKET,@object # @SDLK_RIGHTBRACKET
	.section	.data.SDLK_RIGHTBRACKET,"aGw",@progbits,SDLK_RIGHTBRACKET,comdat
	.weak	SDLK_RIGHTBRACKET
	.align	4
SDLK_RIGHTBRACKET:
	.long	93                      # 0x5d
	.size	SDLK_RIGHTBRACKET, 4

	.type	SDLK_CARET,@object      # @SDLK_CARET
	.section	.data.SDLK_CARET,"aGw",@progbits,SDLK_CARET,comdat
	.weak	SDLK_CARET
	.align	4
SDLK_CARET:
	.long	94                      # 0x5e
	.size	SDLK_CARET, 4

	.type	SDLK_UNDERSCORE,@object # @SDLK_UNDERSCORE
	.section	.data.SDLK_UNDERSCORE,"aGw",@progbits,SDLK_UNDERSCORE,comdat
	.weak	SDLK_UNDERSCORE
	.align	4
SDLK_UNDERSCORE:
	.long	95                      # 0x5f
	.size	SDLK_UNDERSCORE, 4

	.type	SDLK_BACKQUOTE,@object  # @SDLK_BACKQUOTE
	.section	.data.SDLK_BACKQUOTE,"aGw",@progbits,SDLK_BACKQUOTE,comdat
	.weak	SDLK_BACKQUOTE
	.align	4
SDLK_BACKQUOTE:
	.long	96                      # 0x60
	.size	SDLK_BACKQUOTE, 4

	.type	SDLK_a,@object          # @SDLK_a
	.section	.data.SDLK_a,"aGw",@progbits,SDLK_a,comdat
	.weak	SDLK_a
	.align	4
SDLK_a:
	.long	97                      # 0x61
	.size	SDLK_a, 4

	.type	SDLK_b,@object          # @SDLK_b
	.section	.data.SDLK_b,"aGw",@progbits,SDLK_b,comdat
	.weak	SDLK_b
	.align	4
SDLK_b:
	.long	98                      # 0x62
	.size	SDLK_b, 4

	.type	SDLK_c,@object          # @SDLK_c
	.section	.data.SDLK_c,"aGw",@progbits,SDLK_c,comdat
	.weak	SDLK_c
	.align	4
SDLK_c:
	.long	99                      # 0x63
	.size	SDLK_c, 4

	.type	SDLK_d,@object          # @SDLK_d
	.section	.data.SDLK_d,"aGw",@progbits,SDLK_d,comdat
	.weak	SDLK_d
	.align	4
SDLK_d:
	.long	100                     # 0x64
	.size	SDLK_d, 4

	.type	SDLK_e,@object          # @SDLK_e
	.section	.data.SDLK_e,"aGw",@progbits,SDLK_e,comdat
	.weak	SDLK_e
	.align	4
SDLK_e:
	.long	101                     # 0x65
	.size	SDLK_e, 4

	.type	SDLK_f,@object          # @SDLK_f
	.section	.data.SDLK_f,"aGw",@progbits,SDLK_f,comdat
	.weak	SDLK_f
	.align	4
SDLK_f:
	.long	102                     # 0x66
	.size	SDLK_f, 4

	.type	SDLK_g,@object          # @SDLK_g
	.section	.data.SDLK_g,"aGw",@progbits,SDLK_g,comdat
	.weak	SDLK_g
	.align	4
SDLK_g:
	.long	103                     # 0x67
	.size	SDLK_g, 4

	.type	SDLK_h,@object          # @SDLK_h
	.section	.data.SDLK_h,"aGw",@progbits,SDLK_h,comdat
	.weak	SDLK_h
	.align	4
SDLK_h:
	.long	104                     # 0x68
	.size	SDLK_h, 4

	.type	SDLK_i,@object          # @SDLK_i
	.section	.data.SDLK_i,"aGw",@progbits,SDLK_i,comdat
	.weak	SDLK_i
	.align	4
SDLK_i:
	.long	105                     # 0x69
	.size	SDLK_i, 4

	.type	SDLK_j,@object          # @SDLK_j
	.section	.data.SDLK_j,"aGw",@progbits,SDLK_j,comdat
	.weak	SDLK_j
	.align	4
SDLK_j:
	.long	106                     # 0x6a
	.size	SDLK_j, 4

	.type	SDLK_k,@object          # @SDLK_k
	.section	.data.SDLK_k,"aGw",@progbits,SDLK_k,comdat
	.weak	SDLK_k
	.align	4
SDLK_k:
	.long	107                     # 0x6b
	.size	SDLK_k, 4

	.type	SDLK_l,@object          # @SDLK_l
	.section	.data.SDLK_l,"aGw",@progbits,SDLK_l,comdat
	.weak	SDLK_l
	.align	4
SDLK_l:
	.long	108                     # 0x6c
	.size	SDLK_l, 4

	.type	SDLK_m,@object          # @SDLK_m
	.section	.data.SDLK_m,"aGw",@progbits,SDLK_m,comdat
	.weak	SDLK_m
	.align	4
SDLK_m:
	.long	109                     # 0x6d
	.size	SDLK_m, 4

	.type	SDLK_n,@object          # @SDLK_n
	.section	.data.SDLK_n,"aGw",@progbits,SDLK_n,comdat
	.weak	SDLK_n
	.align	4
SDLK_n:
	.long	110                     # 0x6e
	.size	SDLK_n, 4

	.type	SDLK_o,@object          # @SDLK_o
	.section	.data.SDLK_o,"aGw",@progbits,SDLK_o,comdat
	.weak	SDLK_o
	.align	4
SDLK_o:
	.long	111                     # 0x6f
	.size	SDLK_o, 4

	.type	SDLK_p,@object          # @SDLK_p
	.section	.data.SDLK_p,"aGw",@progbits,SDLK_p,comdat
	.weak	SDLK_p
	.align	4
SDLK_p:
	.long	112                     # 0x70
	.size	SDLK_p, 4

	.type	SDLK_q,@object          # @SDLK_q
	.section	.data.SDLK_q,"aGw",@progbits,SDLK_q,comdat
	.weak	SDLK_q
	.align	4
SDLK_q:
	.long	113                     # 0x71
	.size	SDLK_q, 4

	.type	SDLK_r,@object          # @SDLK_r
	.section	.data.SDLK_r,"aGw",@progbits,SDLK_r,comdat
	.weak	SDLK_r
	.align	4
SDLK_r:
	.long	114                     # 0x72
	.size	SDLK_r, 4

	.type	SDLK_s,@object          # @SDLK_s
	.section	.data.SDLK_s,"aGw",@progbits,SDLK_s,comdat
	.weak	SDLK_s
	.align	4
SDLK_s:
	.long	115                     # 0x73
	.size	SDLK_s, 4

	.type	SDLK_t,@object          # @SDLK_t
	.section	.data.SDLK_t,"aGw",@progbits,SDLK_t,comdat
	.weak	SDLK_t
	.align	4
SDLK_t:
	.long	116                     # 0x74
	.size	SDLK_t, 4

	.type	SDLK_u,@object          # @SDLK_u
	.section	.data.SDLK_u,"aGw",@progbits,SDLK_u,comdat
	.weak	SDLK_u
	.align	4
SDLK_u:
	.long	117                     # 0x75
	.size	SDLK_u, 4

	.type	SDLK_v,@object          # @SDLK_v
	.section	.data.SDLK_v,"aGw",@progbits,SDLK_v,comdat
	.weak	SDLK_v
	.align	4
SDLK_v:
	.long	118                     # 0x76
	.size	SDLK_v, 4

	.type	SDLK_w,@object          # @SDLK_w
	.section	.data.SDLK_w,"aGw",@progbits,SDLK_w,comdat
	.weak	SDLK_w
	.align	4
SDLK_w:
	.long	119                     # 0x77
	.size	SDLK_w, 4

	.type	SDLK_x,@object          # @SDLK_x
	.section	.data.SDLK_x,"aGw",@progbits,SDLK_x,comdat
	.weak	SDLK_x
	.align	4
SDLK_x:
	.long	120                     # 0x78
	.size	SDLK_x, 4

	.type	SDLK_y,@object          # @SDLK_y
	.section	.data.SDLK_y,"aGw",@progbits,SDLK_y,comdat
	.weak	SDLK_y
	.align	4
SDLK_y:
	.long	121                     # 0x79
	.size	SDLK_y, 4

	.type	SDLK_z,@object          # @SDLK_z
	.section	.data.SDLK_z,"aGw",@progbits,SDLK_z,comdat
	.weak	SDLK_z
	.align	4
SDLK_z:
	.long	122                     # 0x7a
	.size	SDLK_z, 4

	.type	SDLK_DELETE,@object     # @SDLK_DELETE
	.section	.data.SDLK_DELETE,"aGw",@progbits,SDLK_DELETE,comdat
	.weak	SDLK_DELETE
	.align	4
SDLK_DELETE:
	.long	127                     # 0x7f
	.size	SDLK_DELETE, 4

	.type	SDLK_UP,@object         # @SDLK_UP
	.section	.data.SDLK_UP,"aGw",@progbits,SDLK_UP,comdat
	.weak	SDLK_UP
	.align	4
SDLK_UP:
	.long	273                     # 0x111
	.size	SDLK_UP, 4

	.type	SDLK_DOWN,@object       # @SDLK_DOWN
	.section	.data.SDLK_DOWN,"aGw",@progbits,SDLK_DOWN,comdat
	.weak	SDLK_DOWN
	.align	4
SDLK_DOWN:
	.long	274                     # 0x112
	.size	SDLK_DOWN, 4

	.type	SDLK_RIGHT,@object      # @SDLK_RIGHT
	.section	.data.SDLK_RIGHT,"aGw",@progbits,SDLK_RIGHT,comdat
	.weak	SDLK_RIGHT
	.align	4
SDLK_RIGHT:
	.long	275                     # 0x113
	.size	SDLK_RIGHT, 4

	.type	SDLK_LEFT,@object       # @SDLK_LEFT
	.section	.data.SDLK_LEFT,"aGw",@progbits,SDLK_LEFT,comdat
	.weak	SDLK_LEFT
	.align	4
SDLK_LEFT:
	.long	276                     # 0x114
	.size	SDLK_LEFT, 4

	.type	SDLK_INSERT,@object     # @SDLK_INSERT
	.section	.data.SDLK_INSERT,"aGw",@progbits,SDLK_INSERT,comdat
	.weak	SDLK_INSERT
	.align	4
SDLK_INSERT:
	.long	277                     # 0x115
	.size	SDLK_INSERT, 4

	.type	SDLK_HOME,@object       # @SDLK_HOME
	.section	.data.SDLK_HOME,"aGw",@progbits,SDLK_HOME,comdat
	.weak	SDLK_HOME
	.align	4
SDLK_HOME:
	.long	278                     # 0x116
	.size	SDLK_HOME, 4

	.type	SDLK_END,@object        # @SDLK_END
	.section	.data.SDLK_END,"aGw",@progbits,SDLK_END,comdat
	.weak	SDLK_END
	.align	4
SDLK_END:
	.long	279                     # 0x117
	.size	SDLK_END, 4

	.type	SDLK_PAGEUP,@object     # @SDLK_PAGEUP
	.section	.data.SDLK_PAGEUP,"aGw",@progbits,SDLK_PAGEUP,comdat
	.weak	SDLK_PAGEUP
	.align	4
SDLK_PAGEUP:
	.long	280                     # 0x118
	.size	SDLK_PAGEUP, 4

	.type	SDLK_PAGEDOWN,@object   # @SDLK_PAGEDOWN
	.section	.data.SDLK_PAGEDOWN,"aGw",@progbits,SDLK_PAGEDOWN,comdat
	.weak	SDLK_PAGEDOWN
	.align	4
SDLK_PAGEDOWN:
	.long	281                     # 0x119
	.size	SDLK_PAGEDOWN, 4

	.type	KMOD_NONE,@object       # @KMOD_NONE
	.section	.bss.KMOD_NONE,"aGw",@nobits,KMOD_NONE,comdat
	.weak	KMOD_NONE
	.align	4
KMOD_NONE:
	.long	0                       # 0x0
	.size	KMOD_NONE, 4

	.type	KMOD_LSHIFT,@object     # @KMOD_LSHIFT
	.section	.data.KMOD_LSHIFT,"aGw",@progbits,KMOD_LSHIFT,comdat
	.weak	KMOD_LSHIFT
	.align	4
KMOD_LSHIFT:
	.long	1                       # 0x1
	.size	KMOD_LSHIFT, 4

	.type	KMOD_RSHIFT,@object     # @KMOD_RSHIFT
	.section	.data.KMOD_RSHIFT,"aGw",@progbits,KMOD_RSHIFT,comdat
	.weak	KMOD_RSHIFT
	.align	4
KMOD_RSHIFT:
	.long	2                       # 0x2
	.size	KMOD_RSHIFT, 4

	.type	KMOD_LCTRL,@object      # @KMOD_LCTRL
	.section	.data.KMOD_LCTRL,"aGw",@progbits,KMOD_LCTRL,comdat
	.weak	KMOD_LCTRL
	.align	4
KMOD_LCTRL:
	.long	64                      # 0x40
	.size	KMOD_LCTRL, 4

	.type	KMOD_RCTRL,@object      # @KMOD_RCTRL
	.section	.data.KMOD_RCTRL,"aGw",@progbits,KMOD_RCTRL,comdat
	.weak	KMOD_RCTRL
	.align	4
KMOD_RCTRL:
	.long	128                     # 0x80
	.size	KMOD_RCTRL, 4

	.type	KMOD_LALT,@object       # @KMOD_LALT
	.section	.data.KMOD_LALT,"aGw",@progbits,KMOD_LALT,comdat
	.weak	KMOD_LALT
	.align	4
KMOD_LALT:
	.long	256                     # 0x100
	.size	KMOD_LALT, 4

	.type	KMOD_RALT,@object       # @KMOD_RALT
	.section	.data.KMOD_RALT,"aGw",@progbits,KMOD_RALT,comdat
	.weak	KMOD_RALT
	.align	4
KMOD_RALT:
	.long	512                     # 0x200
	.size	KMOD_RALT, 4

	.type	MIX_INIT_FLAC,@object   # @MIX_INIT_FLAC
	.section	.data.MIX_INIT_FLAC,"aGw",@progbits,MIX_INIT_FLAC,comdat
	.weak	MIX_INIT_FLAC
	.align	4
MIX_INIT_FLAC:
	.long	1                       # 0x1
	.size	MIX_INIT_FLAC, 4

	.type	MIX_INIT_MOD,@object    # @MIX_INIT_MOD
	.section	.data.MIX_INIT_MOD,"aGw",@progbits,MIX_INIT_MOD,comdat
	.weak	MIX_INIT_MOD
	.align	4
MIX_INIT_MOD:
	.long	2                       # 0x2
	.size	MIX_INIT_MOD, 4

	.type	MIX_INIT_MP3,@object    # @MIX_INIT_MP3
	.section	.data.MIX_INIT_MP3,"aGw",@progbits,MIX_INIT_MP3,comdat
	.weak	MIX_INIT_MP3
	.align	4
MIX_INIT_MP3:
	.long	4                       # 0x4
	.size	MIX_INIT_MP3, 4

	.type	MIX_INIT_OGG,@object    # @MIX_INIT_OGG
	.section	.data.MIX_INIT_OGG,"aGw",@progbits,MIX_INIT_OGG,comdat
	.weak	MIX_INIT_OGG
	.align	4
MIX_INIT_OGG:
	.long	8                       # 0x8
	.size	MIX_INIT_OGG, 4

	.type	MIX_CHANNELS,@object    # @MIX_CHANNELS
	.section	.data.MIX_CHANNELS,"aGw",@progbits,MIX_CHANNELS,comdat
	.weak	MIX_CHANNELS
	.align	4
MIX_CHANNELS:
	.long	8                       # 0x8
	.size	MIX_CHANNELS, 4

	.type	MIX_NO_FADING,@object   # @MIX_NO_FADING
	.section	.bss.MIX_NO_FADING,"aGw",@nobits,MIX_NO_FADING,comdat
	.weak	MIX_NO_FADING
	.align	4
MIX_NO_FADING:
	.long	0                       # 0x0
	.size	MIX_NO_FADING, 4

	.type	MIX_FADING_OUT,@object  # @MIX_FADING_OUT
	.section	.data.MIX_FADING_OUT,"aGw",@progbits,MIX_FADING_OUT,comdat
	.weak	MIX_FADING_OUT
	.align	4
MIX_FADING_OUT:
	.long	1                       # 0x1
	.size	MIX_FADING_OUT, 4

	.type	MIX_FADING_IN,@object   # @MIX_FADING_IN
	.section	.data.MIX_FADING_IN,"aGw",@progbits,MIX_FADING_IN,comdat
	.weak	MIX_FADING_IN
	.align	4
MIX_FADING_IN:
	.long	2                       # 0x2
	.size	MIX_FADING_IN, 4

	.type	SDL_BUTTON_LEFT,@object # @SDL_BUTTON_LEFT
	.section	.data.SDL_BUTTON_LEFT,"aGw",@progbits,SDL_BUTTON_LEFT,comdat
	.weak	SDL_BUTTON_LEFT
	.align	4
SDL_BUTTON_LEFT:
	.long	1                       # 0x1
	.size	SDL_BUTTON_LEFT, 4

	.type	SDL_BUTTON_MIDDLE,@object # @SDL_BUTTON_MIDDLE
	.section	.data.SDL_BUTTON_MIDDLE,"aGw",@progbits,SDL_BUTTON_MIDDLE,comdat
	.weak	SDL_BUTTON_MIDDLE
	.align	4
SDL_BUTTON_MIDDLE:
	.long	2                       # 0x2
	.size	SDL_BUTTON_MIDDLE, 4

	.type	SDL_BUTTON_RIGHT,@object # @SDL_BUTTON_RIGHT
	.section	.data.SDL_BUTTON_RIGHT,"aGw",@progbits,SDL_BUTTON_RIGHT,comdat
	.weak	SDL_BUTTON_RIGHT
	.align	4
SDL_BUTTON_RIGHT:
	.long	3                       # 0x3
	.size	SDL_BUTTON_RIGHT, 4

	.type	SDL_SWSURFACE,@object   # @SDL_SWSURFACE
	.section	.bss.SDL_SWSURFACE,"aGw",@nobits,SDL_SWSURFACE,comdat
	.weak	SDL_SWSURFACE
	.align	4
SDL_SWSURFACE:
	.long	0                       # 0x0
	.size	SDL_SWSURFACE, 4

	.type	RAND_MAX,@object        # @RAND_MAX
	.section	.data.RAND_MAX,"aGw",@progbits,RAND_MAX,comdat
	.weak	RAND_MAX
	.align	4
RAND_MAX:
	.long	214783647               # 0xccd569f
	.size	RAND_MAX, 4

	.type	EXIT_FAILURE,@object    # @EXIT_FAILURE
	.section	.data.EXIT_FAILURE,"aGw",@progbits,EXIT_FAILURE,comdat
	.weak	EXIT_FAILURE
	.align	4
EXIT_FAILURE:
	.long	1                       # 0x1
	.size	EXIT_FAILURE, 4

	.type	EXIT_SUCCESS,@object    # @EXIT_SUCCESS
	.section	.bss.EXIT_SUCCESS,"aGw",@nobits,EXIT_SUCCESS,comdat
	.weak	EXIT_SUCCESS
	.align	4
EXIT_SUCCESS:
	.long	0                       # 0x0
	.size	EXIT_SUCCESS, 4

	.type	EOF,@object             # @EOF
	.section	.data.EOF,"aGw",@progbits,EOF,comdat
	.weak	EOF
	.align	4
EOF:
	.long	4294967295              # 0xffffffff
	.size	EOF, 4

	.type	.L__unnamed_2,@object   # @1
	.section	.rodata,"a",@progbits
.L__unnamed_2:
	.asciz	"/tmp"
	.size	.L__unnamed_2, 5

	.type	P_tmpdir,@object        # @P_tmpdir
	.section	.data.P_tmpdir,"aGw",@progbits,P_tmpdir,comdat
	.weak	P_tmpdir
	.align	8
P_tmpdir:
	.quad	.L__unnamed_2
	.size	P_tmpdir, 8

	.type	SEEK_SET,@object        # @SEEK_SET
	.section	.bss.SEEK_SET,"aGw",@nobits,SEEK_SET,comdat
	.weak	SEEK_SET
	.align	4
SEEK_SET:
	.long	0                       # 0x0
	.size	SEEK_SET, 4

	.type	SEEK_CUR,@object        # @SEEK_CUR
	.section	.data.SEEK_CUR,"aGw",@progbits,SEEK_CUR,comdat
	.weak	SEEK_CUR
	.align	4
SEEK_CUR:
	.long	1                       # 0x1
	.size	SEEK_CUR, 4

	.type	SEEK_END,@object        # @SEEK_END
	.section	.data.SEEK_END,"aGw",@progbits,SEEK_END,comdat
	.weak	SEEK_END
	.align	4
SEEK_END:
	.long	2                       # 0x2
	.size	SEEK_END, 4

	.type	stdin,@object           # @stdin
	.bss
	.globl	stdin
	.align	8
stdin:
	.quad	0
	.size	stdin, 8

	.type	stdout,@object          # @stdout
	.globl	stdout
	.align	8
stdout:
	.quad	0
	.size	stdout, 8

	.type	stderr,@object          # @stderr
	.globl	stderr
	.align	8
stderr:
	.quad	0
	.size	stderr, 8

	.section	.bss.EXIT_SUCCESS,"aGw",@nobits,EXIT_SUCCESS,comdat
.Ldebug_end0:
	.section	.bss.KMOD_NONE,"aGw",@nobits,KMOD_NONE,comdat
.Ldebug_end1:
	.section	.bss.MIX_NO_FADING,"aGw",@nobits,MIX_NO_FADING,comdat
.Ldebug_end2:
	.section	.bss.SDL_AUDIO_STOPPED,"aGw",@nobits,SDL_AUDIO_STOPPED,comdat
.Ldebug_end3:
	.section	.bss.SDL_SWSURFACE,"aGw",@nobits,SDL_SWSURFACE,comdat
.Ldebug_end4:
	.section	.bss.SEEK_SET,"aGw",@nobits,SEEK_SET,comdat
.Ldebug_end5:
	.section	.bss.back,"aGw",@nobits,back,comdat
.Ldebug_end6:
	.section	.bss.surf,"aGw",@nobits,surf,comdat
.Ldebug_end7:
	.section	.bss.testval,"aGw",@nobits,testval,comdat
.Ldebug_end8:
	.bss
.Ldebug_end9:
	.section	.data.AUDIO_S16LSB,"aGw",@progbits,AUDIO_S16LSB,comdat
.Ldebug_end10:
	.section	.data.AUDIO_S8,"aGw",@progbits,AUDIO_S8,comdat
.Ldebug_end11:
	.section	.data.AUDIO_U16LSB,"aGw",@progbits,AUDIO_U16LSB,comdat
.Ldebug_end12:
	.section	.data.AUDIO_U8,"aGw",@progbits,AUDIO_U8,comdat
.Ldebug_end13:
	.section	.data.EOF,"aGw",@progbits,EOF,comdat
.Ldebug_end14:
	.section	.data.EXIT_FAILURE,"aGw",@progbits,EXIT_FAILURE,comdat
.Ldebug_end15:
	.section	.data.IMG_INIT_JPG,"aGw",@progbits,IMG_INIT_JPG,comdat
.Ldebug_end16:
	.section	.data.IMG_INIT_PNG,"aGw",@progbits,IMG_INIT_PNG,comdat
.Ldebug_end17:
	.section	.data.IMG_INIT_TIF,"aGw",@progbits,IMG_INIT_TIF,comdat
.Ldebug_end18:
	.section	.data.IMG_INIT_WEBP,"aGw",@progbits,IMG_INIT_WEBP,comdat
.Ldebug_end19:
	.section	.data.KMOD_LALT,"aGw",@progbits,KMOD_LALT,comdat
.Ldebug_end20:
	.section	.data.KMOD_LCTRL,"aGw",@progbits,KMOD_LCTRL,comdat
.Ldebug_end21:
	.section	.data.KMOD_LSHIFT,"aGw",@progbits,KMOD_LSHIFT,comdat
.Ldebug_end22:
	.section	.data.KMOD_RALT,"aGw",@progbits,KMOD_RALT,comdat
.Ldebug_end23:
	.section	.data.KMOD_RCTRL,"aGw",@progbits,KMOD_RCTRL,comdat
.Ldebug_end24:
	.section	.data.KMOD_RSHIFT,"aGw",@progbits,KMOD_RSHIFT,comdat
.Ldebug_end25:
	.section	.data.MIX_CHANNELS,"aGw",@progbits,MIX_CHANNELS,comdat
.Ldebug_end26:
	.section	.data.MIX_FADING_IN,"aGw",@progbits,MIX_FADING_IN,comdat
.Ldebug_end27:
	.section	.data.MIX_FADING_OUT,"aGw",@progbits,MIX_FADING_OUT,comdat
.Ldebug_end28:
	.section	.data.MIX_INIT_FLAC,"aGw",@progbits,MIX_INIT_FLAC,comdat
.Ldebug_end29:
	.section	.data.MIX_INIT_MOD,"aGw",@progbits,MIX_INIT_MOD,comdat
.Ldebug_end30:
	.section	.data.MIX_INIT_MP3,"aGw",@progbits,MIX_INIT_MP3,comdat
.Ldebug_end31:
	.section	.data.MIX_INIT_OGG,"aGw",@progbits,MIX_INIT_OGG,comdat
.Ldebug_end32:
	.section	.data.P_tmpdir,"aGw",@progbits,P_tmpdir,comdat
.Ldebug_end33:
	.section	.data.RAND_MAX1,"aGw",@progbits,RAND_MAX1,comdat
.Ldebug_end34:
	.section	.data.RAND_MAX,"aGw",@progbits,RAND_MAX,comdat
.Ldebug_end35:
	.section	.data.SDLK_0,"aGw",@progbits,SDLK_0,comdat
.Ldebug_end36:
	.section	.data.SDLK_1,"aGw",@progbits,SDLK_1,comdat
.Ldebug_end37:
	.section	.data.SDLK_2,"aGw",@progbits,SDLK_2,comdat
.Ldebug_end38:
	.section	.data.SDLK_3,"aGw",@progbits,SDLK_3,comdat
.Ldebug_end39:
	.section	.data.SDLK_4,"aGw",@progbits,SDLK_4,comdat
.Ldebug_end40:
	.section	.data.SDLK_5,"aGw",@progbits,SDLK_5,comdat
.Ldebug_end41:
	.section	.data.SDLK_6,"aGw",@progbits,SDLK_6,comdat
.Ldebug_end42:
	.section	.data.SDLK_7,"aGw",@progbits,SDLK_7,comdat
.Ldebug_end43:
	.section	.data.SDLK_8,"aGw",@progbits,SDLK_8,comdat
.Ldebug_end44:
	.section	.data.SDLK_9,"aGw",@progbits,SDLK_9,comdat
.Ldebug_end45:
	.section	.data.SDLK_AMPERSAND,"aGw",@progbits,SDLK_AMPERSAND,comdat
.Ldebug_end46:
	.section	.data.SDLK_ASTERISK,"aGw",@progbits,SDLK_ASTERISK,comdat
.Ldebug_end47:
	.section	.data.SDLK_AT,"aGw",@progbits,SDLK_AT,comdat
.Ldebug_end48:
	.section	.data.SDLK_BACKQUOTE,"aGw",@progbits,SDLK_BACKQUOTE,comdat
.Ldebug_end49:
	.section	.data.SDLK_BACKSLASH,"aGw",@progbits,SDLK_BACKSLASH,comdat
.Ldebug_end50:
	.section	.data.SDLK_BACKSPACE,"aGw",@progbits,SDLK_BACKSPACE,comdat
.Ldebug_end51:
	.section	.data.SDLK_CARET,"aGw",@progbits,SDLK_CARET,comdat
.Ldebug_end52:
	.section	.data.SDLK_CLEAR,"aGw",@progbits,SDLK_CLEAR,comdat
.Ldebug_end53:
	.section	.data.SDLK_COLON,"aGw",@progbits,SDLK_COLON,comdat
.Ldebug_end54:
	.section	.data.SDLK_COMMA,"aGw",@progbits,SDLK_COMMA,comdat
.Ldebug_end55:
	.section	.data.SDLK_DELETE,"aGw",@progbits,SDLK_DELETE,comdat
.Ldebug_end56:
	.section	.data.SDLK_DOLLAR,"aGw",@progbits,SDLK_DOLLAR,comdat
.Ldebug_end57:
	.section	.data.SDLK_DOWN,"aGw",@progbits,SDLK_DOWN,comdat
.Ldebug_end58:
	.section	.data.SDLK_END,"aGw",@progbits,SDLK_END,comdat
.Ldebug_end59:
	.section	.data.SDLK_EQUALS,"aGw",@progbits,SDLK_EQUALS,comdat
.Ldebug_end60:
	.section	.data.SDLK_ESCAPE,"aGw",@progbits,SDLK_ESCAPE,comdat
.Ldebug_end61:
	.section	.data.SDLK_EXCLAIM,"aGw",@progbits,SDLK_EXCLAIM,comdat
.Ldebug_end62:
	.section	.data.SDLK_GREATER,"aGw",@progbits,SDLK_GREATER,comdat
.Ldebug_end63:
	.section	.data.SDLK_HASH,"aGw",@progbits,SDLK_HASH,comdat
.Ldebug_end64:
	.section	.data.SDLK_HOME,"aGw",@progbits,SDLK_HOME,comdat
.Ldebug_end65:
	.section	.data.SDLK_INSERT,"aGw",@progbits,SDLK_INSERT,comdat
.Ldebug_end66:
	.section	.data.SDLK_LEFTBRACKET,"aGw",@progbits,SDLK_LEFTBRACKET,comdat
.Ldebug_end67:
	.section	.data.SDLK_LEFTPAREN,"aGw",@progbits,SDLK_LEFTPAREN,comdat
.Ldebug_end68:
	.section	.data.SDLK_LEFT,"aGw",@progbits,SDLK_LEFT,comdat
.Ldebug_end69:
	.section	.data.SDLK_LESS,"aGw",@progbits,SDLK_LESS,comdat
.Ldebug_end70:
	.section	.data.SDLK_MINUS,"aGw",@progbits,SDLK_MINUS,comdat
.Ldebug_end71:
	.section	.data.SDLK_PAGEDOWN,"aGw",@progbits,SDLK_PAGEDOWN,comdat
.Ldebug_end72:
	.section	.data.SDLK_PAGEUP,"aGw",@progbits,SDLK_PAGEUP,comdat
.Ldebug_end73:
	.section	.data.SDLK_PAUSE,"aGw",@progbits,SDLK_PAUSE,comdat
.Ldebug_end74:
	.section	.data.SDLK_PERIOD,"aGw",@progbits,SDLK_PERIOD,comdat
.Ldebug_end75:
	.section	.data.SDLK_PLUS,"aGw",@progbits,SDLK_PLUS,comdat
.Ldebug_end76:
	.section	.data.SDLK_QUESTION,"aGw",@progbits,SDLK_QUESTION,comdat
.Ldebug_end77:
	.section	.data.SDLK_QUOTEDBL,"aGw",@progbits,SDLK_QUOTEDBL,comdat
.Ldebug_end78:
	.section	.data.SDLK_QUOTE,"aGw",@progbits,SDLK_QUOTE,comdat
.Ldebug_end79:
	.section	.data.SDLK_RETURN,"aGw",@progbits,SDLK_RETURN,comdat
.Ldebug_end80:
	.section	.data.SDLK_RIGHTBRACKET,"aGw",@progbits,SDLK_RIGHTBRACKET,comdat
.Ldebug_end81:
	.section	.data.SDLK_RIGHTPAREN,"aGw",@progbits,SDLK_RIGHTPAREN,comdat
.Ldebug_end82:
	.section	.data.SDLK_RIGHT,"aGw",@progbits,SDLK_RIGHT,comdat
.Ldebug_end83:
	.section	.data.SDLK_SEMICOLON,"aGw",@progbits,SDLK_SEMICOLON,comdat
.Ldebug_end84:
	.section	.data.SDLK_SLASH,"aGw",@progbits,SDLK_SLASH,comdat
.Ldebug_end85:
	.section	.data.SDLK_SPACE,"aGw",@progbits,SDLK_SPACE,comdat
.Ldebug_end86:
	.section	.data.SDLK_TAB,"aGw",@progbits,SDLK_TAB,comdat
.Ldebug_end87:
	.section	.data.SDLK_UNDERSCORE,"aGw",@progbits,SDLK_UNDERSCORE,comdat
.Ldebug_end88:
	.section	.data.SDLK_UP,"aGw",@progbits,SDLK_UP,comdat
.Ldebug_end89:
	.section	.data.SDLK_a,"aGw",@progbits,SDLK_a,comdat
.Ldebug_end90:
	.section	.data.SDLK_b,"aGw",@progbits,SDLK_b,comdat
.Ldebug_end91:
	.section	.data.SDLK_c,"aGw",@progbits,SDLK_c,comdat
.Ldebug_end92:
	.section	.data.SDLK_d,"aGw",@progbits,SDLK_d,comdat
.Ldebug_end93:
	.section	.data.SDLK_e,"aGw",@progbits,SDLK_e,comdat
.Ldebug_end94:
	.section	.data.SDLK_f,"aGw",@progbits,SDLK_f,comdat
.Ldebug_end95:
	.section	.data.SDLK_g,"aGw",@progbits,SDLK_g,comdat
.Ldebug_end96:
	.section	.data.SDLK_h,"aGw",@progbits,SDLK_h,comdat
.Ldebug_end97:
	.section	.data.SDLK_i,"aGw",@progbits,SDLK_i,comdat
.Ldebug_end98:
	.section	.data.SDLK_j,"aGw",@progbits,SDLK_j,comdat
.Ldebug_end99:
	.section	.data.SDLK_k,"aGw",@progbits,SDLK_k,comdat
.Ldebug_end100:
	.section	.data.SDLK_l,"aGw",@progbits,SDLK_l,comdat
.Ldebug_end101:
	.section	.data.SDLK_m,"aGw",@progbits,SDLK_m,comdat
.Ldebug_end102:
	.section	.data.SDLK_n,"aGw",@progbits,SDLK_n,comdat
.Ldebug_end103:
	.section	.data.SDLK_o,"aGw",@progbits,SDLK_o,comdat
.Ldebug_end104:
	.section	.data.SDLK_p,"aGw",@progbits,SDLK_p,comdat
.Ldebug_end105:
	.section	.data.SDLK_q,"aGw",@progbits,SDLK_q,comdat
.Ldebug_end106:
	.section	.data.SDLK_r,"aGw",@progbits,SDLK_r,comdat
.Ldebug_end107:
	.section	.data.SDLK_s,"aGw",@progbits,SDLK_s,comdat
.Ldebug_end108:
	.section	.data.SDLK_t,"aGw",@progbits,SDLK_t,comdat
.Ldebug_end109:
	.section	.data.SDLK_u,"aGw",@progbits,SDLK_u,comdat
.Ldebug_end110:
	.section	.data.SDLK_v,"aGw",@progbits,SDLK_v,comdat
.Ldebug_end111:
	.section	.data.SDLK_w,"aGw",@progbits,SDLK_w,comdat
.Ldebug_end112:
	.section	.data.SDLK_x,"aGw",@progbits,SDLK_x,comdat
.Ldebug_end113:
	.section	.data.SDLK_y,"aGw",@progbits,SDLK_y,comdat
.Ldebug_end114:
	.section	.data.SDLK_z,"aGw",@progbits,SDLK_z,comdat
.Ldebug_end115:
	.section	.data.SDL_APPACTIVE,"aGw",@progbits,SDL_APPACTIVE,comdat
.Ldebug_end116:
	.section	.data.SDL_APPINPUTFOCUS,"aGw",@progbits,SDL_APPINPUTFOCUS,comdat
.Ldebug_end117:
	.section	.data.SDL_APPMOUSEFOCUS,"aGw",@progbits,SDL_APPMOUSEFOCUS,comdat
.Ldebug_end118:
	.section	.data.SDL_AUDIO_PAUSED,"aGw",@progbits,SDL_AUDIO_PAUSED,comdat
.Ldebug_end119:
	.section	.data.SDL_AUDIO_PLAYING,"aGw",@progbits,SDL_AUDIO_PLAYING,comdat
.Ldebug_end120:
	.section	.data.SDL_BUTTON_LEFT,"aGw",@progbits,SDL_BUTTON_LEFT,comdat
.Ldebug_end121:
	.section	.data.SDL_BUTTON_MIDDLE,"aGw",@progbits,SDL_BUTTON_MIDDLE,comdat
.Ldebug_end122:
	.section	.data.SDL_BUTTON_RIGHT,"aGw",@progbits,SDL_BUTTON_RIGHT,comdat
.Ldebug_end123:
	.section	.data.SDL_DEFAULT_REPEAT_DELAY,"aGw",@progbits,SDL_DEFAULT_REPEAT_DELAY,comdat
.Ldebug_end124:
	.section	.data.SDL_DEFAULT_REPEAT_INTERVAL,"aGw",@progbits,SDL_DEFAULT_REPEAT_INTERVAL,comdat
.Ldebug_end125:
	.section	.data.SDL_INIT_AUDIO,"aGw",@progbits,SDL_INIT_AUDIO,comdat
.Ldebug_end126:
	.section	.data.SDL_INIT_CDROM,"aGw",@progbits,SDL_INIT_CDROM,comdat
.Ldebug_end127:
	.section	.data.SDL_INIT_EVERYTHING,"aGw",@progbits,SDL_INIT_EVERYTHING,comdat
.Ldebug_end128:
	.section	.data.SDL_INIT_JOYSTICK,"aGw",@progbits,SDL_INIT_JOYSTICK,comdat
.Ldebug_end129:
	.section	.data.SDL_INIT_TIMER,"aGw",@progbits,SDL_INIT_TIMER,comdat
.Ldebug_end130:
	.section	.data.SDL_INIT_VIDEO,"aGw",@progbits,SDL_INIT_VIDEO,comdat
.Ldebug_end131:
	.section	.data.SEEK_CUR,"aGw",@progbits,SEEK_CUR,comdat
.Ldebug_end132:
	.section	.data.SEEK_END,"aGw",@progbits,SEEK_END,comdat
.Ldebug_end133:
	.text
.Ldebug_end134:
	.section	.debug_str,"MS",@progbits,1
.Linfo_string0:
	.asciz	"wlc 0.11 - Jan 2014 (GGJ)"
.Linfo_string1:
	.asciz	"test.wl"
.Linfo_string2:
	.asciz	"/home/brandon/PROJECTS/C/term/wlc"
.Linfo_string3:
	.asciz	"RAND_MAX"
.Linfo_string4:
	.asciz	"int32"
.Linfo_string5:
	.asciz	"surf"
.Linfo_string6:
	.asciz	"flags"
.Linfo_string7:
	.asciz	"format"
.Linfo_string8:
	.asciz	"palette"
.Linfo_string9:
	.asciz	"ncolors"
.Linfo_string10:
	.asciz	"colors"
.Linfo_string11:
	.asciz	"r"
.Linfo_string12:
	.asciz	"char"
.Linfo_string13:
	.asciz	"g"
.Linfo_string14:
	.asciz	"b"
.Linfo_string15:
	.asciz	"unused"
.Linfo_string16:
	.asciz	"SDL_Color"
.Linfo_string17:
	.asciz	"SDL_Palette"
.Linfo_string18:
	.asciz	"BitsPerPixel"
.Linfo_string19:
	.asciz	"uchar"
.Linfo_string20:
	.asciz	"BytesPerPixel"
.Linfo_string21:
	.asciz	"Rloss"
.Linfo_string22:
	.asciz	"Gloss"
.Linfo_string23:
	.asciz	"Bloss"
.Linfo_string24:
	.asciz	"Aloss"
.Linfo_string25:
	.asciz	"Rshift"
.Linfo_string26:
	.asciz	"Gshift"
.Linfo_string27:
	.asciz	"Bshift"
.Linfo_string28:
	.asciz	"Ashift"
.Linfo_string29:
	.asciz	"Rmask"
.Linfo_string30:
	.asciz	"uint32"
.Linfo_string31:
	.asciz	"Gmask"
.Linfo_string32:
	.asciz	"Bmask"
.Linfo_string33:
	.asciz	"Amask"
.Linfo_string34:
	.asciz	"SDL_PixelFormat"
.Linfo_string35:
	.asciz	"w"
.Linfo_string36:
	.asciz	"h"
.Linfo_string37:
	.asciz	"pitch"
.Linfo_string38:
	.asciz	"int16"
.Linfo_string39:
	.asciz	"pixels"
.Linfo_string40:
	.asciz	"offset"
.Linfo_string41:
	.asciz	"hwdata"
.Linfo_string42:
	.asciz	"clip_rect"
.Linfo_string43:
	.asciz	"x"
.Linfo_string44:
	.asciz	"y"
.Linfo_string45:
	.asciz	"uint16"
.Linfo_string46:
	.asciz	"SDL_Rect"
.Linfo_string47:
	.asciz	"unused1"
.Linfo_string48:
	.asciz	"locked"
.Linfo_string49:
	.asciz	"map"
.Linfo_string50:
	.asciz	"format_version"
.Linfo_string51:
	.asciz	"refcount"
.Linfo_string52:
	.asciz	"SDL_Surface"
.Linfo_string53:
	.asciz	"back"
.Linfo_string54:
	.asciz	"testval"
.Linfo_string55:
	.asciz	"pixelIsWhite"
.Linfo_string56:
	.asciz	"bool"
.Linfo_string57:
	.asciz	"setPixel"
.Linfo_string58:
	.asciz	"void"
.Linfo_string59:
	.asciz	"update"
.Linfo_string60:
	.asciz	"randomize"
.Linfo_string61:
	.asciz	"somefunc"
.Linfo_string62:
	.asciz	"main"
.Linfo_string63:
	.asciz	"sdl.wl"
.Linfo_string64:
	.asciz	"SDL_INIT_TIMER"
.Linfo_string65:
	.asciz	"SDL_INIT_AUDIO"
.Linfo_string66:
	.asciz	"SDL_INIT_VIDEO"
.Linfo_string67:
	.asciz	"SDL_INIT_CDROM"
.Linfo_string68:
	.asciz	"SDL_INIT_JOYSTICK"
.Linfo_string69:
	.asciz	"SDL_INIT_EVERYTHING"
.Linfo_string70:
	.asciz	"SDL_APPMOUSEFOCUS"
.Linfo_string71:
	.asciz	"SDL_APPINPUTFOCUS"
.Linfo_string72:
	.asciz	"SDL_APPACTIVE"
.Linfo_string73:
	.asciz	"AUDIO_U8"
.Linfo_string74:
	.asciz	"AUDIO_S8"
.Linfo_string75:
	.asciz	"AUDIO_U16LSB"
.Linfo_string76:
	.asciz	"AUDIO_S16LSB"
.Linfo_string77:
	.asciz	"SDL_AUDIO_STOPPED"
.Linfo_string78:
	.asciz	"SDL_AUDIO_PLAYING"
.Linfo_string79:
	.asciz	"SDL_AUDIO_PAUSED"
.Linfo_string80:
	.asciz	"IMG_INIT_JPG"
.Linfo_string81:
	.asciz	"IMG_INIT_PNG"
.Linfo_string82:
	.asciz	"IMG_INIT_TIF"
.Linfo_string83:
	.asciz	"IMG_INIT_WEBP"
.Linfo_string84:
	.asciz	"SDL_DEFAULT_REPEAT_DELAY"
.Linfo_string85:
	.asciz	"SDL_DEFAULT_REPEAT_INTERVAL"
.Linfo_string86:
	.asciz	"SDLK_BACKSPACE"
.Linfo_string87:
	.asciz	"SDLK_TAB"
.Linfo_string88:
	.asciz	"SDLK_CLEAR"
.Linfo_string89:
	.asciz	"SDLK_RETURN"
.Linfo_string90:
	.asciz	"SDLK_PAUSE"
.Linfo_string91:
	.asciz	"SDLK_ESCAPE"
.Linfo_string92:
	.asciz	"SDLK_SPACE"
.Linfo_string93:
	.asciz	"SDLK_EXCLAIM"
.Linfo_string94:
	.asciz	"SDLK_QUOTEDBL"
.Linfo_string95:
	.asciz	"SDLK_HASH"
.Linfo_string96:
	.asciz	"SDLK_DOLLAR"
.Linfo_string97:
	.asciz	"SDLK_AMPERSAND"
.Linfo_string98:
	.asciz	"SDLK_QUOTE"
.Linfo_string99:
	.asciz	"SDLK_LEFTPAREN"
.Linfo_string100:
	.asciz	"SDLK_RIGHTPAREN"
.Linfo_string101:
	.asciz	"SDLK_ASTERISK"
.Linfo_string102:
	.asciz	"SDLK_PLUS"
.Linfo_string103:
	.asciz	"SDLK_COMMA"
.Linfo_string104:
	.asciz	"SDLK_MINUS"
.Linfo_string105:
	.asciz	"SDLK_PERIOD"
.Linfo_string106:
	.asciz	"SDLK_SLASH"
.Linfo_string107:
	.asciz	"SDLK_0"
.Linfo_string108:
	.asciz	"SDLK_1"
.Linfo_string109:
	.asciz	"SDLK_2"
.Linfo_string110:
	.asciz	"SDLK_3"
.Linfo_string111:
	.asciz	"SDLK_4"
.Linfo_string112:
	.asciz	"SDLK_5"
.Linfo_string113:
	.asciz	"SDLK_6"
.Linfo_string114:
	.asciz	"SDLK_7"
.Linfo_string115:
	.asciz	"SDLK_8"
.Linfo_string116:
	.asciz	"SDLK_9"
.Linfo_string117:
	.asciz	"SDLK_COLON"
.Linfo_string118:
	.asciz	"SDLK_SEMICOLON"
.Linfo_string119:
	.asciz	"SDLK_LESS"
.Linfo_string120:
	.asciz	"SDLK_EQUALS"
.Linfo_string121:
	.asciz	"SDLK_GREATER"
.Linfo_string122:
	.asciz	"SDLK_QUESTION"
.Linfo_string123:
	.asciz	"SDLK_AT"
.Linfo_string124:
	.asciz	"SDLK_LEFTBRACKET"
.Linfo_string125:
	.asciz	"SDLK_BACKSLASH"
.Linfo_string126:
	.asciz	"SDLK_RIGHTBRACKET"
.Linfo_string127:
	.asciz	"SDLK_CARET"
.Linfo_string128:
	.asciz	"SDLK_UNDERSCORE"
.Linfo_string129:
	.asciz	"SDLK_BACKQUOTE"
.Linfo_string130:
	.asciz	"SDLK_a"
.Linfo_string131:
	.asciz	"SDLK_b"
.Linfo_string132:
	.asciz	"SDLK_c"
.Linfo_string133:
	.asciz	"SDLK_d"
.Linfo_string134:
	.asciz	"SDLK_e"
.Linfo_string135:
	.asciz	"SDLK_f"
.Linfo_string136:
	.asciz	"SDLK_g"
.Linfo_string137:
	.asciz	"SDLK_h"
.Linfo_string138:
	.asciz	"SDLK_i"
.Linfo_string139:
	.asciz	"SDLK_j"
.Linfo_string140:
	.asciz	"SDLK_k"
.Linfo_string141:
	.asciz	"SDLK_l"
.Linfo_string142:
	.asciz	"SDLK_m"
.Linfo_string143:
	.asciz	"SDLK_n"
.Linfo_string144:
	.asciz	"SDLK_o"
.Linfo_string145:
	.asciz	"SDLK_p"
.Linfo_string146:
	.asciz	"SDLK_q"
.Linfo_string147:
	.asciz	"SDLK_r"
.Linfo_string148:
	.asciz	"SDLK_s"
.Linfo_string149:
	.asciz	"SDLK_t"
.Linfo_string150:
	.asciz	"SDLK_u"
.Linfo_string151:
	.asciz	"SDLK_v"
.Linfo_string152:
	.asciz	"SDLK_w"
.Linfo_string153:
	.asciz	"SDLK_x"
.Linfo_string154:
	.asciz	"SDLK_y"
.Linfo_string155:
	.asciz	"SDLK_z"
.Linfo_string156:
	.asciz	"SDLK_DELETE"
.Linfo_string157:
	.asciz	"SDLK_UP"
.Linfo_string158:
	.asciz	"SDLK_DOWN"
.Linfo_string159:
	.asciz	"SDLK_RIGHT"
.Linfo_string160:
	.asciz	"SDLK_LEFT"
.Linfo_string161:
	.asciz	"SDLK_INSERT"
.Linfo_string162:
	.asciz	"SDLK_HOME"
.Linfo_string163:
	.asciz	"SDLK_END"
.Linfo_string164:
	.asciz	"SDLK_PAGEUP"
.Linfo_string165:
	.asciz	"SDLK_PAGEDOWN"
.Linfo_string166:
	.asciz	"KMOD_NONE"
.Linfo_string167:
	.asciz	"KMOD_LSHIFT"
.Linfo_string168:
	.asciz	"KMOD_RSHIFT"
.Linfo_string169:
	.asciz	"KMOD_LCTRL"
.Linfo_string170:
	.asciz	"KMOD_RCTRL"
.Linfo_string171:
	.asciz	"KMOD_LALT"
.Linfo_string172:
	.asciz	"KMOD_RALT"
.Linfo_string173:
	.asciz	"MIX_INIT_FLAC"
.Linfo_string174:
	.asciz	"MIX_INIT_MOD"
.Linfo_string175:
	.asciz	"MIX_INIT_MP3"
.Linfo_string176:
	.asciz	"MIX_INIT_OGG"
.Linfo_string177:
	.asciz	"MIX_CHANNELS"
.Linfo_string178:
	.asciz	"MIX_NO_FADING"
.Linfo_string179:
	.asciz	"MIX_FADING_OUT"
.Linfo_string180:
	.asciz	"MIX_FADING_IN"
.Linfo_string181:
	.asciz	"SDL_BUTTON_LEFT"
.Linfo_string182:
	.asciz	"SDL_BUTTON_MIDDLE"
.Linfo_string183:
	.asciz	"SDL_BUTTON_RIGHT"
.Linfo_string184:
	.asciz	"SDL_SWSURFACE"
.Linfo_string185:
	.asciz	"Mix_PlayChannel"
.Linfo_string186:
	.asciz	"cstdlib.wl"
.Linfo_string187:
	.asciz	"EXIT_FAILURE"
.Linfo_string188:
	.asciz	"EXIT_SUCCESS"
.Linfo_string189:
	.asciz	"assert"
.Linfo_string190:
	.asciz	"cstdio.wl"
.Linfo_string191:
	.asciz	"EOF"
.Linfo_string192:
	.asciz	"P_tmpdir"
.Linfo_string193:
	.asciz	"SEEK_SET"
.Linfo_string194:
	.asciz	"SEEK_CUR"
.Linfo_string195:
	.asciz	"SEEK_END"
.Linfo_string196:
	.asciz	"stdin"
.Linfo_string197:
	.asciz	"_IO_FILE"
.Linfo_string198:
	.asciz	"stdout"
.Linfo_string199:
	.asciz	"stderr"
.Linfo_string200:
	.asciz	"s"
.Linfo_string201:
	.asciz	"i"
.Linfo_string202:
	.asciz	"j"
.Linfo_string203:
	.asciz	"ret"
.Linfo_string204:
	.asciz	"val"
.Linfo_string205:
	.asciz	"n"
.Linfo_string206:
	.asciz	"argc"
.Linfo_string207:
	.asciz	"argv"
.Linfo_string208:
	.asciz	"title"
.Linfo_string209:
	.asciz	"iii"
.Linfo_string210:
	.asciz	"float32"
.Linfo_string211:
	.asciz	"spc"
.Linfo_string212:
	.asciz	"chan"
.Linfo_string213:
	.asciz	"chunk"
.Linfo_string214:
	.asciz	"allocated"
.Linfo_string215:
	.asciz	"abuf"
.Linfo_string216:
	.asciz	"alen"
.Linfo_string217:
	.asciz	"volume"
.Linfo_string218:
	.asciz	"Mix_Chunk"
.Linfo_string219:
	.asciz	"loops"
.Linfo_string220:
	.asciz	"cnd"
.Linfo_string221:
	.asciz	"err"
	.section	.debug_info,"",@progbits
.L.debug_info_begin0:
	.long	1398                    # Length of Unit
	.short	4                       # DWARF version number
	.long	.L.debug_abbrev_begin   # Offset Into Abbrev. Section
	.byte	8                       # Address Size (in bytes)
	.byte	1                       # Abbrev [1] 0xb:0x56f DW_TAG_compile_unit
	.long	.Linfo_string0          # DW_AT_producer
	.short	0                       # DW_AT_language
	.long	.Linfo_string1          # DW_AT_name
	.quad	0                       # DW_AT_low_pc
	.long	.Lsection_line          # DW_AT_stmt_list
	.long	.Linfo_string2          # DW_AT_comp_dir
	.byte	2                       # Abbrev [2] 0x26:0x19 DW_TAG_variable
	.long	.Linfo_string3          # DW_AT_name
	.long	63                      # DW_AT_type
                                        # DW_AT_external
	.byte	1                       # DW_AT_decl_file
	.byte	5                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	RAND_MAX1
	.long	.Linfo_string3          # DW_AT_MIPS_linkage_name
	.byte	3                       # Abbrev [3] 0x3f:0x7 DW_TAG_base_type
	.long	.Linfo_string4          # DW_AT_name
	.byte	5                       # DW_AT_encoding
	.byte	4                       # DW_AT_byte_size
	.byte	2                       # Abbrev [2] 0x46:0x19 DW_TAG_variable
	.long	.Linfo_string5          # DW_AT_name
	.long	95                      # DW_AT_type
                                        # DW_AT_external
	.byte	1                       # DW_AT_decl_file
	.byte	10                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	surf
	.long	.Linfo_string5          # DW_AT_MIPS_linkage_name
	.byte	4                       # Abbrev [4] 0x5f:0x5 DW_TAG_pointer_type
	.long	100                     # DW_AT_type
	.byte	5                       # Abbrev [5] 0x64:0xfa DW_TAG_structure_type
	.long	.Linfo_string52         # DW_AT_name
	.byte	9                       # DW_AT_byte_size
	.byte	1                       # DW_AT_decl_file
	.short	339                     # DW_AT_decl_line
	.byte	6                       # Abbrev [6] 0x6d:0x11 DW_TAG_member
	.long	.Linfo_string6          # DW_AT_name
	.long	63                      # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0x7e:0x11 DW_TAG_member
	.long	.Linfo_string7          # DW_AT_name
	.long	350                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	8                       # DW_AT_byte_size
	.byte	8                       # DW_AT_bit_size
	.byte	55                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0x8f:0x11 DW_TAG_member
	.long	.Linfo_string35         # DW_AT_name
	.long	63                      # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0xa0:0x11 DW_TAG_member
	.long	.Linfo_string36         # DW_AT_name
	.long	63                      # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0xb1:0x11 DW_TAG_member
	.long	.Linfo_string37         # DW_AT_name
	.long	773                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	2                       # DW_AT_byte_size
	.byte	2                       # DW_AT_bit_size
	.byte	13                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0xc2:0x11 DW_TAG_member
	.long	.Linfo_string39         # DW_AT_name
	.long	780                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	8                       # DW_AT_byte_size
	.byte	8                       # DW_AT_bit_size
	.byte	55                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	6                       # Abbrev [6] 0xd3:0x11 DW_TAG_member
	.long	.Linfo_string40         # DW_AT_name
	.long	63                      # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	7                       # Abbrev [7] 0xe4:0x12 DW_TAG_member
	.long	.Linfo_string41         # DW_AT_name
	.long	780                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	8                       # DW_AT_byte_size
	.byte	8                       # DW_AT_bit_size
	.byte	55                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	1                       # DW_AT_virtuality
	.byte	8                       # Abbrev [8] 0xf6:0xf DW_TAG_member
	.long	.Linfo_string42         # DW_AT_name
	.long	785                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	1                       # DW_AT_virtuality
	.byte	7                       # Abbrev [7] 0x105:0x12 DW_TAG_member
	.long	.Linfo_string47         # DW_AT_name
	.long	63                      # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	1                       # DW_AT_virtuality
	.byte	7                       # Abbrev [7] 0x117:0x12 DW_TAG_member
	.long	.Linfo_string48         # DW_AT_name
	.long	63                      # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	1                       # DW_AT_virtuality
	.byte	7                       # Abbrev [7] 0x129:0x12 DW_TAG_member
	.long	.Linfo_string49         # DW_AT_name
	.long	780                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	8                       # DW_AT_byte_size
	.byte	8                       # DW_AT_bit_size
	.byte	55                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	1                       # DW_AT_virtuality
	.byte	9                       # Abbrev [9] 0x13b:0x11 DW_TAG_member
	.long	.Linfo_string50         # DW_AT_name
	.long	766                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
                                        # DW_AT_artificial
	.byte	9                       # Abbrev [9] 0x14c:0x11 DW_TAG_member
	.long	.Linfo_string51         # DW_AT_name
	.long	63                      # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	341                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
                                        # DW_AT_artificial
	.byte	0                       # End Of Children Mark
	.byte	4                       # Abbrev [4] 0x15e:0x5 DW_TAG_pointer_type
	.long	355                     # DW_AT_type
	.byte	5                       # Abbrev [5] 0x163:0x109 DW_TAG_structure_type
	.long	.Linfo_string34         # DW_AT_name
	.byte	4                       # DW_AT_byte_size
	.byte	1                       # DW_AT_decl_file
	.short	318                     # DW_AT_decl_line
	.byte	6                       # Abbrev [6] 0x16c:0x11 DW_TAG_member
	.long	.Linfo_string8          # DW_AT_name
	.long	620                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	8                       # DW_AT_byte_size
	.byte	8                       # DW_AT_bit_size
	.byte	55                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0x17d:0x11 DW_TAG_member
	.long	.Linfo_string18         # DW_AT_name
	.long	759                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0x18e:0x11 DW_TAG_member
	.long	.Linfo_string20         # DW_AT_name
	.long	759                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	3                       # DW_AT_accessibility
                                        # DW_ACCESS_private
	.byte	6                       # Abbrev [6] 0x19f:0x11 DW_TAG_member
	.long	.Linfo_string21         # DW_AT_name
	.long	759                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	6                       # Abbrev [6] 0x1b0:0x11 DW_TAG_member
	.long	.Linfo_string22         # DW_AT_name
	.long	759                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	6                       # Abbrev [6] 0x1c1:0x11 DW_TAG_member
	.long	.Linfo_string23         # DW_AT_name
	.long	759                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0x1d2:0x11 DW_TAG_member
	.long	.Linfo_string24         # DW_AT_name
	.long	759                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	3                       # DW_AT_accessibility
                                        # DW_ACCESS_private
	.byte	6                       # Abbrev [6] 0x1e3:0x11 DW_TAG_member
	.long	.Linfo_string25         # DW_AT_name
	.long	759                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	6                       # Abbrev [6] 0x1f4:0x11 DW_TAG_member
	.long	.Linfo_string26         # DW_AT_name
	.long	759                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	6                       # Abbrev [6] 0x205:0x11 DW_TAG_member
	.long	.Linfo_string27         # DW_AT_name
	.long	759                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0x216:0x11 DW_TAG_member
	.long	.Linfo_string28         # DW_AT_name
	.long	759                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	3                       # DW_AT_accessibility
                                        # DW_ACCESS_private
	.byte	6                       # Abbrev [6] 0x227:0x11 DW_TAG_member
	.long	.Linfo_string29         # DW_AT_name
	.long	766                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	6                       # Abbrev [6] 0x238:0x11 DW_TAG_member
	.long	.Linfo_string31         # DW_AT_name
	.long	766                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	6                       # Abbrev [6] 0x249:0x11 DW_TAG_member
	.long	.Linfo_string32         # DW_AT_name
	.long	766                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	6                       # Abbrev [6] 0x25a:0x11 DW_TAG_member
	.long	.Linfo_string33         # DW_AT_name
	.long	766                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	320                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	0                       # End Of Children Mark
	.byte	4                       # Abbrev [4] 0x26c:0x5 DW_TAG_pointer_type
	.long	625                     # DW_AT_type
	.byte	5                       # Abbrev [5] 0x271:0x2c DW_TAG_structure_type
	.long	.Linfo_string17         # DW_AT_name
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_decl_file
	.short	312                     # DW_AT_decl_line
	.byte	6                       # Abbrev [6] 0x27a:0x11 DW_TAG_member
	.long	.Linfo_string9          # DW_AT_name
	.long	63                      # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	314                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0x28b:0x11 DW_TAG_member
	.long	.Linfo_string10         # DW_AT_name
	.long	669                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	314                     # DW_AT_decl_line
	.byte	8                       # DW_AT_byte_size
	.byte	8                       # DW_AT_bit_size
	.byte	55                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	0                       # End Of Children Mark
	.byte	4                       # Abbrev [4] 0x29d:0x5 DW_TAG_pointer_type
	.long	674                     # DW_AT_type
	.byte	5                       # Abbrev [5] 0x2a2:0x4e DW_TAG_structure_type
	.long	.Linfo_string16         # DW_AT_name
	.byte	0                       # DW_AT_byte_size
	.byte	1                       # DW_AT_decl_file
	.short	304                     # DW_AT_decl_line
	.byte	6                       # Abbrev [6] 0x2ab:0x11 DW_TAG_member
	.long	.Linfo_string11         # DW_AT_name
	.long	752                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	306                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0x2bc:0x11 DW_TAG_member
	.long	.Linfo_string13         # DW_AT_name
	.long	752                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	306                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	3                       # DW_AT_accessibility
                                        # DW_ACCESS_private
	.byte	6                       # Abbrev [6] 0x2cd:0x11 DW_TAG_member
	.long	.Linfo_string14         # DW_AT_name
	.long	752                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	306                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	6                       # Abbrev [6] 0x2de:0x11 DW_TAG_member
	.long	.Linfo_string15         # DW_AT_name
	.long	752                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	306                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	0                       # End Of Children Mark
	.byte	3                       # Abbrev [3] 0x2f0:0x7 DW_TAG_base_type
	.long	.Linfo_string12         # DW_AT_name
	.byte	6                       # DW_AT_encoding
	.byte	1                       # DW_AT_byte_size
	.byte	3                       # Abbrev [3] 0x2f7:0x7 DW_TAG_base_type
	.long	.Linfo_string19         # DW_AT_name
	.byte	8                       # DW_AT_encoding
	.byte	1                       # DW_AT_byte_size
	.byte	3                       # Abbrev [3] 0x2fe:0x7 DW_TAG_base_type
	.long	.Linfo_string30         # DW_AT_name
	.byte	7                       # DW_AT_encoding
	.byte	4                       # DW_AT_byte_size
	.byte	3                       # Abbrev [3] 0x305:0x7 DW_TAG_base_type
	.long	.Linfo_string38         # DW_AT_name
	.byte	5                       # DW_AT_encoding
	.byte	2                       # DW_AT_byte_size
	.byte	4                       # Abbrev [4] 0x30c:0x5 DW_TAG_pointer_type
	.long	752                     # DW_AT_type
	.byte	5                       # Abbrev [5] 0x311:0x4e DW_TAG_structure_type
	.long	.Linfo_string46         # DW_AT_name
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_decl_file
	.short	261                     # DW_AT_decl_line
	.byte	6                       # Abbrev [6] 0x31a:0x11 DW_TAG_member
	.long	.Linfo_string43         # DW_AT_name
	.long	773                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	263                     # DW_AT_decl_line
	.byte	2                       # DW_AT_byte_size
	.byte	2                       # DW_AT_bit_size
	.byte	13                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0x32b:0x11 DW_TAG_member
	.long	.Linfo_string44         # DW_AT_name
	.long	773                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	263                     # DW_AT_decl_line
	.byte	2                       # DW_AT_byte_size
	.byte	2                       # DW_AT_bit_size
	.byte	13                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	6                       # Abbrev [6] 0x33c:0x11 DW_TAG_member
	.long	.Linfo_string35         # DW_AT_name
	.long	863                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	263                     # DW_AT_decl_line
	.byte	2                       # DW_AT_byte_size
	.byte	2                       # DW_AT_bit_size
	.byte	13                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	6                       # Abbrev [6] 0x34d:0x11 DW_TAG_member
	.long	.Linfo_string36         # DW_AT_name
	.long	863                     # DW_AT_type
	.byte	1                       # DW_AT_decl_file
	.short	263                     # DW_AT_decl_line
	.byte	2                       # DW_AT_byte_size
	.byte	2                       # DW_AT_bit_size
	.byte	13                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	2                       # DW_AT_accessibility
                                        # DW_ACCESS_protected
	.byte	0                       # End Of Children Mark
	.byte	3                       # Abbrev [3] 0x35f:0x7 DW_TAG_base_type
	.long	.Linfo_string45         # DW_AT_name
	.byte	7                       # DW_AT_encoding
	.byte	2                       # DW_AT_byte_size
	.byte	2                       # Abbrev [2] 0x366:0x19 DW_TAG_variable
	.long	.Linfo_string53         # DW_AT_name
	.long	95                      # DW_AT_type
                                        # DW_AT_external
	.byte	1                       # DW_AT_decl_file
	.byte	11                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	back
	.long	.Linfo_string53         # DW_AT_MIPS_linkage_name
	.byte	2                       # Abbrev [2] 0x37f:0x19 DW_TAG_variable
	.long	.Linfo_string54         # DW_AT_name
	.long	780                     # DW_AT_type
                                        # DW_AT_external
	.byte	1                       # DW_AT_decl_file
	.byte	12                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	testval
	.long	.Linfo_string54         # DW_AT_MIPS_linkage_name
	.byte	10                      # Abbrev [10] 0x398:0x5a DW_TAG_subprogram
	.long	.Linfo_string55         # DW_AT_MIPS_linkage_name
	.long	.Linfo_string55         # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	20                      # DW_AT_decl_line
	.long	1010                    # DW_AT_type
                                        # DW_AT_external
	.quad	.Lfunc_begin0           # DW_AT_low_pc
	.quad	.Lfunc_end0             # DW_AT_high_pc
	.byte	1                       # DW_AT_frame_base
	.byte	86
	.byte	11                      # Abbrev [11] 0x3b9:0xe DW_TAG_formal_parameter
	.long	.Linfo_string200        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	20                      # DW_AT_decl_line
	.long	95                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	112
	.byte	11                      # Abbrev [11] 0x3c7:0xe DW_TAG_formal_parameter
	.long	.Linfo_string201        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	20                      # DW_AT_decl_line
	.long	63                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	104
	.byte	11                      # Abbrev [11] 0x3d5:0xe DW_TAG_formal_parameter
	.long	.Linfo_string202        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	20                      # DW_AT_decl_line
	.long	63                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	96
	.byte	12                      # Abbrev [12] 0x3e3:0xe DW_TAG_variable
	.long	.Linfo_string203        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	22                      # DW_AT_decl_line
	.long	1010                    # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	88
	.byte	0                       # End Of Children Mark
	.byte	3                       # Abbrev [3] 0x3f2:0x7 DW_TAG_base_type
	.long	.Linfo_string56         # DW_AT_name
	.byte	2                       # DW_AT_encoding
	.byte	1                       # DW_AT_byte_size
	.byte	10                      # Abbrev [10] 0x3f9:0x5a DW_TAG_subprogram
	.long	.Linfo_string57         # DW_AT_MIPS_linkage_name
	.long	.Linfo_string57         # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	26                      # DW_AT_decl_line
	.long	1107                    # DW_AT_type
                                        # DW_AT_external
	.quad	.Lfunc_begin1           # DW_AT_low_pc
	.quad	.Lfunc_end1             # DW_AT_high_pc
	.byte	1                       # DW_AT_frame_base
	.byte	86
	.byte	11                      # Abbrev [11] 0x41a:0xe DW_TAG_formal_parameter
	.long	.Linfo_string200        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	26                      # DW_AT_decl_line
	.long	95                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	120
	.byte	11                      # Abbrev [11] 0x428:0xe DW_TAG_formal_parameter
	.long	.Linfo_string201        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	26                      # DW_AT_decl_line
	.long	63                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	112
	.byte	11                      # Abbrev [11] 0x436:0xe DW_TAG_formal_parameter
	.long	.Linfo_string202        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	26                      # DW_AT_decl_line
	.long	63                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	104
	.byte	11                      # Abbrev [11] 0x444:0xe DW_TAG_formal_parameter
	.long	.Linfo_string204        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	26                      # DW_AT_decl_line
	.long	63                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	96
	.byte	0                       # End Of Children Mark
	.byte	3                       # Abbrev [3] 0x453:0x7 DW_TAG_base_type
	.long	.Linfo_string58         # DW_AT_name
	.byte	1                       # DW_AT_encoding
	.byte	1                       # DW_AT_byte_size
	.byte	10                      # Abbrev [10] 0x45a:0x3e DW_TAG_subprogram
	.long	.Linfo_string59         # DW_AT_MIPS_linkage_name
	.long	.Linfo_string59         # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	31                      # DW_AT_decl_line
	.long	1107                    # DW_AT_type
                                        # DW_AT_external
	.quad	.Lfunc_begin2           # DW_AT_low_pc
	.quad	.Lfunc_end2             # DW_AT_high_pc
	.byte	1                       # DW_AT_frame_base
	.byte	86
	.byte	12                      # Abbrev [12] 0x47b:0xe DW_TAG_variable
	.long	.Linfo_string205        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	33                      # DW_AT_decl_line
	.long	63                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	112
	.byte	12                      # Abbrev [12] 0x489:0xe DW_TAG_variable
	.long	.Linfo_string202        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	34                      # DW_AT_decl_line
	.long	63                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	104
	.byte	0                       # End Of Children Mark
	.byte	10                      # Abbrev [10] 0x498:0x3e DW_TAG_subprogram
	.long	.Linfo_string60         # DW_AT_MIPS_linkage_name
	.long	.Linfo_string60         # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	56                      # DW_AT_decl_line
	.long	1107                    # DW_AT_type
                                        # DW_AT_external
	.quad	.Lfunc_begin3           # DW_AT_low_pc
	.quad	.Lfunc_end3             # DW_AT_high_pc
	.byte	1                       # DW_AT_frame_base
	.byte	86
	.byte	11                      # Abbrev [11] 0x4b9:0xe DW_TAG_formal_parameter
	.long	.Linfo_string200        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	56                      # DW_AT_decl_line
	.long	95                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	88
	.byte	12                      # Abbrev [12] 0x4c7:0xe DW_TAG_variable
	.long	.Linfo_string202        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	58                      # DW_AT_decl_line
	.long	63                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	80
	.byte	0                       # End Of Children Mark
	.byte	13                      # Abbrev [13] 0x4d6:0x21 DW_TAG_subprogram
	.long	.Linfo_string61         # DW_AT_MIPS_linkage_name
	.long	.Linfo_string61         # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	69                      # DW_AT_decl_line
	.long	780                     # DW_AT_type
                                        # DW_AT_external
	.quad	.Lfunc_begin4           # DW_AT_low_pc
	.quad	.Lfunc_end4             # DW_AT_high_pc
	.byte	1                       # DW_AT_frame_base
	.byte	86
	.byte	10                      # Abbrev [10] 0x4f7:0x76 DW_TAG_subprogram
	.long	.Linfo_string62         # DW_AT_MIPS_linkage_name
	.long	.Linfo_string62         # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	75                      # DW_AT_decl_line
	.long	63                      # DW_AT_type
                                        # DW_AT_external
	.quad	.Lfunc_begin5           # DW_AT_low_pc
	.quad	.Lfunc_end5             # DW_AT_high_pc
	.byte	1                       # DW_AT_frame_base
	.byte	86
	.byte	11                      # Abbrev [11] 0x518:0xe DW_TAG_formal_parameter
	.long	.Linfo_string206        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	75                      # DW_AT_decl_line
	.long	63                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	112
	.byte	11                      # Abbrev [11] 0x526:0xe DW_TAG_formal_parameter
	.long	.Linfo_string207        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	75                      # DW_AT_decl_line
	.long	1389                    # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	104
	.byte	12                      # Abbrev [12] 0x534:0xe DW_TAG_variable
	.long	.Linfo_string208        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	84                      # DW_AT_decl_line
	.long	780                     # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	96
	.byte	12                      # Abbrev [12] 0x542:0xe DW_TAG_variable
	.long	.Linfo_string209        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	87                      # DW_AT_decl_line
	.long	63                      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	88
	.byte	12                      # Abbrev [12] 0x550:0xe DW_TAG_variable
	.long	.Linfo_string202        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	88                      # DW_AT_decl_line
	.long	1394                    # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	80
	.byte	12                      # Abbrev [12] 0x55e:0xe DW_TAG_variable
	.long	.Linfo_string211        # DW_AT_name
	.byte	1                       # DW_AT_decl_file
	.byte	91                      # DW_AT_decl_line
	.long	752                     # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	72
	.byte	0                       # End Of Children Mark
	.byte	4                       # Abbrev [4] 0x56d:0x5 DW_TAG_pointer_type
	.long	780                     # DW_AT_type
	.byte	3                       # Abbrev [3] 0x572:0x7 DW_TAG_base_type
	.long	.Linfo_string210        # DW_AT_name
	.byte	4                       # DW_AT_encoding
	.byte	4                       # DW_AT_byte_size
	.byte	0                       # End Of Children Mark
.L.debug_info_end0:
.L.debug_info_begin1:
	.long	3223                    # Length of Unit
	.short	4                       # DWARF version number
	.long	.L.debug_abbrev_begin   # Offset Into Abbrev. Section
	.byte	8                       # Address Size (in bytes)
	.byte	1                       # Abbrev [1] 0xb:0xc90 DW_TAG_compile_unit
	.long	.Linfo_string0          # DW_AT_producer
	.short	0                       # DW_AT_language
	.long	.Linfo_string63         # DW_AT_name
	.quad	0                       # DW_AT_low_pc
	.long	.Lsection_line          # DW_AT_stmt_list
	.long	.Linfo_string2          # DW_AT_comp_dir
	.byte	14                      # Abbrev [14] 0x26:0x19 DW_TAG_variable
	.long	.Linfo_string64         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	3                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_INIT_TIMER
	.long	.Linfo_string64         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x3f:0x19 DW_TAG_variable
	.long	.Linfo_string65         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	4                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_INIT_AUDIO
	.long	.Linfo_string65         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x58:0x19 DW_TAG_variable
	.long	.Linfo_string66         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	5                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_INIT_VIDEO
	.long	.Linfo_string66         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x71:0x19 DW_TAG_variable
	.long	.Linfo_string67         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	6                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_INIT_CDROM
	.long	.Linfo_string67         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x8a:0x19 DW_TAG_variable
	.long	.Linfo_string68         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	7                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_INIT_JOYSTICK
	.long	.Linfo_string68         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xa3:0x19 DW_TAG_variable
	.long	.Linfo_string69         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	8                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_INIT_EVERYTHING
	.long	.Linfo_string69         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xbc:0x19 DW_TAG_variable
	.long	.Linfo_string70         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	17                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_APPMOUSEFOCUS
	.long	.Linfo_string70         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xd5:0x19 DW_TAG_variable
	.long	.Linfo_string71         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	18                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_APPINPUTFOCUS
	.long	.Linfo_string71         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xee:0x19 DW_TAG_variable
	.long	.Linfo_string72         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	19                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_APPACTIVE
	.long	.Linfo_string72         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x107:0x19 DW_TAG_variable
	.long	.Linfo_string73         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	37                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	AUDIO_U8
	.long	.Linfo_string73         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x120:0x19 DW_TAG_variable
	.long	.Linfo_string74         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	38                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	AUDIO_S8
	.long	.Linfo_string74         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x139:0x19 DW_TAG_variable
	.long	.Linfo_string75         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	39                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	AUDIO_U16LSB
	.long	.Linfo_string75         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x152:0x19 DW_TAG_variable
	.long	.Linfo_string76         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	40                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	AUDIO_S16LSB
	.long	.Linfo_string76         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x16b:0x19 DW_TAG_variable
	.long	.Linfo_string77         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	63                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_AUDIO_STOPPED
	.long	.Linfo_string77         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x184:0x19 DW_TAG_variable
	.long	.Linfo_string78         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	64                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_AUDIO_PLAYING
	.long	.Linfo_string78         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x19d:0x19 DW_TAG_variable
	.long	.Linfo_string79         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	65                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_AUDIO_PAUSED
	.long	.Linfo_string79         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x1b6:0x19 DW_TAG_variable
	.long	.Linfo_string80         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	95                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	IMG_INIT_JPG
	.long	.Linfo_string80         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x1cf:0x19 DW_TAG_variable
	.long	.Linfo_string81         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	96                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	IMG_INIT_PNG
	.long	.Linfo_string81         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x1e8:0x19 DW_TAG_variable
	.long	.Linfo_string82         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	97                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	IMG_INIT_TIF
	.long	.Linfo_string82         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x201:0x19 DW_TAG_variable
	.long	.Linfo_string83         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	98                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	IMG_INIT_WEBP
	.long	.Linfo_string83         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x21a:0x19 DW_TAG_variable
	.long	.Linfo_string84         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	114                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_DEFAULT_REPEAT_DELAY
	.long	.Linfo_string84         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x233:0x19 DW_TAG_variable
	.long	.Linfo_string85         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	115                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_DEFAULT_REPEAT_INTERVAL
	.long	.Linfo_string85         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x24c:0x19 DW_TAG_variable
	.long	.Linfo_string86         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	126                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_BACKSPACE
	.long	.Linfo_string86         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x265:0x19 DW_TAG_variable
	.long	.Linfo_string87         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	127                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_TAB
	.long	.Linfo_string87         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x27e:0x19 DW_TAG_variable
	.long	.Linfo_string88         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	128                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_CLEAR
	.long	.Linfo_string88         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x297:0x19 DW_TAG_variable
	.long	.Linfo_string89         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	129                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_RETURN
	.long	.Linfo_string89         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x2b0:0x19 DW_TAG_variable
	.long	.Linfo_string90         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	130                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_PAUSE
	.long	.Linfo_string90         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x2c9:0x19 DW_TAG_variable
	.long	.Linfo_string91         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	131                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_ESCAPE
	.long	.Linfo_string91         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x2e2:0x19 DW_TAG_variable
	.long	.Linfo_string92         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	132                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_SPACE
	.long	.Linfo_string92         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x2fb:0x19 DW_TAG_variable
	.long	.Linfo_string93         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	133                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_EXCLAIM
	.long	.Linfo_string93         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x314:0x19 DW_TAG_variable
	.long	.Linfo_string94         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	134                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_QUOTEDBL
	.long	.Linfo_string94         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x32d:0x19 DW_TAG_variable
	.long	.Linfo_string95         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	135                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_HASH
	.long	.Linfo_string95         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x346:0x19 DW_TAG_variable
	.long	.Linfo_string96         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	136                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_DOLLAR
	.long	.Linfo_string96         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x35f:0x19 DW_TAG_variable
	.long	.Linfo_string97         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	137                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_AMPERSAND
	.long	.Linfo_string97         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x378:0x19 DW_TAG_variable
	.long	.Linfo_string98         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	138                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_QUOTE
	.long	.Linfo_string98         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x391:0x19 DW_TAG_variable
	.long	.Linfo_string99         # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	139                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_LEFTPAREN
	.long	.Linfo_string99         # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x3aa:0x19 DW_TAG_variable
	.long	.Linfo_string100        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	140                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_RIGHTPAREN
	.long	.Linfo_string100        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x3c3:0x19 DW_TAG_variable
	.long	.Linfo_string101        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	141                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_ASTERISK
	.long	.Linfo_string101        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x3dc:0x19 DW_TAG_variable
	.long	.Linfo_string102        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	142                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_PLUS
	.long	.Linfo_string102        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x3f5:0x19 DW_TAG_variable
	.long	.Linfo_string103        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	143                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_COMMA
	.long	.Linfo_string103        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x40e:0x19 DW_TAG_variable
	.long	.Linfo_string104        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	144                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_MINUS
	.long	.Linfo_string104        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x427:0x19 DW_TAG_variable
	.long	.Linfo_string105        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	145                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_PERIOD
	.long	.Linfo_string105        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x440:0x19 DW_TAG_variable
	.long	.Linfo_string106        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	146                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_SLASH
	.long	.Linfo_string106        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x459:0x19 DW_TAG_variable
	.long	.Linfo_string107        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	147                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_0
	.long	.Linfo_string107        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x472:0x19 DW_TAG_variable
	.long	.Linfo_string108        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	148                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_1
	.long	.Linfo_string108        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x48b:0x19 DW_TAG_variable
	.long	.Linfo_string109        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	149                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_2
	.long	.Linfo_string109        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x4a4:0x19 DW_TAG_variable
	.long	.Linfo_string110        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	150                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_3
	.long	.Linfo_string110        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x4bd:0x19 DW_TAG_variable
	.long	.Linfo_string111        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	151                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_4
	.long	.Linfo_string111        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x4d6:0x19 DW_TAG_variable
	.long	.Linfo_string112        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	152                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_5
	.long	.Linfo_string112        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x4ef:0x19 DW_TAG_variable
	.long	.Linfo_string113        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	153                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_6
	.long	.Linfo_string113        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x508:0x19 DW_TAG_variable
	.long	.Linfo_string114        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	154                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_7
	.long	.Linfo_string114        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x521:0x19 DW_TAG_variable
	.long	.Linfo_string115        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	155                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_8
	.long	.Linfo_string115        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x53a:0x19 DW_TAG_variable
	.long	.Linfo_string116        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	156                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_9
	.long	.Linfo_string116        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x553:0x19 DW_TAG_variable
	.long	.Linfo_string117        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	157                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_COLON
	.long	.Linfo_string117        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x56c:0x19 DW_TAG_variable
	.long	.Linfo_string118        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	158                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_SEMICOLON
	.long	.Linfo_string118        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x585:0x19 DW_TAG_variable
	.long	.Linfo_string119        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	159                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_LESS
	.long	.Linfo_string119        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x59e:0x19 DW_TAG_variable
	.long	.Linfo_string120        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	160                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_EQUALS
	.long	.Linfo_string120        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x5b7:0x19 DW_TAG_variable
	.long	.Linfo_string121        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	161                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_GREATER
	.long	.Linfo_string121        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x5d0:0x19 DW_TAG_variable
	.long	.Linfo_string122        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	162                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_QUESTION
	.long	.Linfo_string122        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x5e9:0x19 DW_TAG_variable
	.long	.Linfo_string123        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	163                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_AT
	.long	.Linfo_string123        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x602:0x19 DW_TAG_variable
	.long	.Linfo_string124        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	165                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_LEFTBRACKET
	.long	.Linfo_string124        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x61b:0x19 DW_TAG_variable
	.long	.Linfo_string125        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	166                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_BACKSLASH
	.long	.Linfo_string125        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x634:0x19 DW_TAG_variable
	.long	.Linfo_string126        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	167                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_RIGHTBRACKET
	.long	.Linfo_string126        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x64d:0x19 DW_TAG_variable
	.long	.Linfo_string127        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	168                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_CARET
	.long	.Linfo_string127        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x666:0x19 DW_TAG_variable
	.long	.Linfo_string128        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	169                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_UNDERSCORE
	.long	.Linfo_string128        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x67f:0x19 DW_TAG_variable
	.long	.Linfo_string129        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	170                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_BACKQUOTE
	.long	.Linfo_string129        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x698:0x19 DW_TAG_variable
	.long	.Linfo_string130        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	171                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_a
	.long	.Linfo_string130        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x6b1:0x19 DW_TAG_variable
	.long	.Linfo_string131        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	172                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_b
	.long	.Linfo_string131        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x6ca:0x19 DW_TAG_variable
	.long	.Linfo_string132        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	173                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_c
	.long	.Linfo_string132        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x6e3:0x19 DW_TAG_variable
	.long	.Linfo_string133        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	174                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_d
	.long	.Linfo_string133        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x6fc:0x19 DW_TAG_variable
	.long	.Linfo_string134        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	175                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_e
	.long	.Linfo_string134        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x715:0x19 DW_TAG_variable
	.long	.Linfo_string135        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	176                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_f
	.long	.Linfo_string135        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x72e:0x19 DW_TAG_variable
	.long	.Linfo_string136        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	177                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_g
	.long	.Linfo_string136        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x747:0x19 DW_TAG_variable
	.long	.Linfo_string137        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	178                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_h
	.long	.Linfo_string137        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x760:0x19 DW_TAG_variable
	.long	.Linfo_string138        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	179                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_i
	.long	.Linfo_string138        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x779:0x19 DW_TAG_variable
	.long	.Linfo_string139        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	180                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_j
	.long	.Linfo_string139        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x792:0x19 DW_TAG_variable
	.long	.Linfo_string140        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	181                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_k
	.long	.Linfo_string140        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x7ab:0x19 DW_TAG_variable
	.long	.Linfo_string141        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	182                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_l
	.long	.Linfo_string141        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x7c4:0x19 DW_TAG_variable
	.long	.Linfo_string142        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	183                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_m
	.long	.Linfo_string142        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x7dd:0x19 DW_TAG_variable
	.long	.Linfo_string143        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	184                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_n
	.long	.Linfo_string143        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x7f6:0x19 DW_TAG_variable
	.long	.Linfo_string144        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	185                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_o
	.long	.Linfo_string144        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x80f:0x19 DW_TAG_variable
	.long	.Linfo_string145        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	186                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_p
	.long	.Linfo_string145        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x828:0x19 DW_TAG_variable
	.long	.Linfo_string146        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	187                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_q
	.long	.Linfo_string146        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x841:0x19 DW_TAG_variable
	.long	.Linfo_string147        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	188                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_r
	.long	.Linfo_string147        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x85a:0x19 DW_TAG_variable
	.long	.Linfo_string148        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	189                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_s
	.long	.Linfo_string148        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x873:0x19 DW_TAG_variable
	.long	.Linfo_string149        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	190                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_t
	.long	.Linfo_string149        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x88c:0x19 DW_TAG_variable
	.long	.Linfo_string150        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	191                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_u
	.long	.Linfo_string150        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x8a5:0x19 DW_TAG_variable
	.long	.Linfo_string151        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	192                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_v
	.long	.Linfo_string151        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x8be:0x19 DW_TAG_variable
	.long	.Linfo_string152        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	193                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_w
	.long	.Linfo_string152        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x8d7:0x19 DW_TAG_variable
	.long	.Linfo_string153        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	194                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_x
	.long	.Linfo_string153        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x8f0:0x19 DW_TAG_variable
	.long	.Linfo_string154        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	195                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_y
	.long	.Linfo_string154        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x909:0x19 DW_TAG_variable
	.long	.Linfo_string155        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	196                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_z
	.long	.Linfo_string155        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x922:0x19 DW_TAG_variable
	.long	.Linfo_string156        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	197                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_DELETE
	.long	.Linfo_string156        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x93b:0x19 DW_TAG_variable
	.long	.Linfo_string157        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	198                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_UP
	.long	.Linfo_string157        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x954:0x19 DW_TAG_variable
	.long	.Linfo_string158        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	199                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_DOWN
	.long	.Linfo_string158        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x96d:0x19 DW_TAG_variable
	.long	.Linfo_string159        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	200                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_RIGHT
	.long	.Linfo_string159        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x986:0x19 DW_TAG_variable
	.long	.Linfo_string160        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	201                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_LEFT
	.long	.Linfo_string160        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x99f:0x19 DW_TAG_variable
	.long	.Linfo_string161        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	202                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_INSERT
	.long	.Linfo_string161        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x9b8:0x19 DW_TAG_variable
	.long	.Linfo_string162        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	203                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_HOME
	.long	.Linfo_string162        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x9d1:0x19 DW_TAG_variable
	.long	.Linfo_string163        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	204                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_END
	.long	.Linfo_string163        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x9ea:0x19 DW_TAG_variable
	.long	.Linfo_string164        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	205                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_PAGEUP
	.long	.Linfo_string164        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xa03:0x19 DW_TAG_variable
	.long	.Linfo_string165        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	206                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDLK_PAGEDOWN
	.long	.Linfo_string165        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xa1c:0x19 DW_TAG_variable
	.long	.Linfo_string166        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	208                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	KMOD_NONE
	.long	.Linfo_string166        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xa35:0x19 DW_TAG_variable
	.long	.Linfo_string167        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	209                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	KMOD_LSHIFT
	.long	.Linfo_string167        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xa4e:0x19 DW_TAG_variable
	.long	.Linfo_string168        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	210                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	KMOD_RSHIFT
	.long	.Linfo_string168        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xa67:0x19 DW_TAG_variable
	.long	.Linfo_string169        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	211                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	KMOD_LCTRL
	.long	.Linfo_string169        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xa80:0x19 DW_TAG_variable
	.long	.Linfo_string170        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	212                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	KMOD_RCTRL
	.long	.Linfo_string170        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xa99:0x19 DW_TAG_variable
	.long	.Linfo_string171        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	213                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	KMOD_LALT
	.long	.Linfo_string171        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xab2:0x19 DW_TAG_variable
	.long	.Linfo_string172        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	214                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	KMOD_RALT
	.long	.Linfo_string172        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xacb:0x19 DW_TAG_variable
	.long	.Linfo_string173        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	217                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	MIX_INIT_FLAC
	.long	.Linfo_string173        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xae4:0x19 DW_TAG_variable
	.long	.Linfo_string174        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	218                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	MIX_INIT_MOD
	.long	.Linfo_string174        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xafd:0x19 DW_TAG_variable
	.long	.Linfo_string175        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	219                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	MIX_INIT_MP3
	.long	.Linfo_string175        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xb16:0x19 DW_TAG_variable
	.long	.Linfo_string176        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	220                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	MIX_INIT_OGG
	.long	.Linfo_string176        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xb2f:0x19 DW_TAG_variable
	.long	.Linfo_string177        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	224                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	MIX_CHANNELS
	.long	.Linfo_string177        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xb48:0x19 DW_TAG_variable
	.long	.Linfo_string178        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	234                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	MIX_NO_FADING
	.long	.Linfo_string178        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xb61:0x19 DW_TAG_variable
	.long	.Linfo_string179        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	235                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	MIX_FADING_OUT
	.long	.Linfo_string179        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0xb7a:0x19 DW_TAG_variable
	.long	.Linfo_string180        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.byte	236                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	MIX_FADING_IN
	.long	.Linfo_string180        # DW_AT_MIPS_linkage_name
	.byte	15                      # Abbrev [15] 0xb93:0x1a DW_TAG_variable
	.long	.Linfo_string181        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.short	290                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_BUTTON_LEFT
	.long	.Linfo_string181        # DW_AT_MIPS_linkage_name
	.byte	15                      # Abbrev [15] 0xbad:0x1a DW_TAG_variable
	.long	.Linfo_string182        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.short	291                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_BUTTON_MIDDLE
	.long	.Linfo_string182        # DW_AT_MIPS_linkage_name
	.byte	15                      # Abbrev [15] 0xbc7:0x1a DW_TAG_variable
	.long	.Linfo_string183        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.short	292                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_BUTTON_RIGHT
	.long	.Linfo_string183        # DW_AT_MIPS_linkage_name
	.byte	15                      # Abbrev [15] 0xbe1:0x1a DW_TAG_variable
	.long	.Linfo_string184        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	2                       # DW_AT_decl_file
	.short	359                     # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SDL_SWSURFACE
	.long	.Linfo_string184        # DW_AT_MIPS_linkage_name
	.byte	16                      # Abbrev [16] 0xbfb:0x4c DW_TAG_subprogram
	.long	.Linfo_string185        # DW_AT_MIPS_linkage_name
	.long	.Linfo_string185        # DW_AT_name
	.byte	2                       # DW_AT_decl_file
	.byte	245                     # DW_AT_decl_line
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.quad	.Lfunc_begin6           # DW_AT_low_pc
	.quad	.Lfunc_end6             # DW_AT_high_pc
	.byte	1                       # DW_AT_frame_base
	.byte	86
	.byte	17                      # Abbrev [17] 0xc1c:0xe DW_TAG_formal_parameter
	.long	.Linfo_string212        # DW_AT_name
	.byte	2                       # DW_AT_decl_file
	.byte	245                     # DW_AT_decl_line
	.long	.Lsection_info+63       # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	120
	.byte	11                      # Abbrev [11] 0xc2a:0xe DW_TAG_formal_parameter
	.long	.Linfo_string213        # DW_AT_name
	.byte	2                       # DW_AT_decl_file
	.byte	245                     # DW_AT_decl_line
	.long	3143                    # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	112
	.byte	17                      # Abbrev [17] 0xc38:0xe DW_TAG_formal_parameter
	.long	.Linfo_string219        # DW_AT_name
	.byte	2                       # DW_AT_decl_file
	.byte	245                     # DW_AT_decl_line
	.long	.Lsection_info+63       # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	104
	.byte	0                       # End Of Children Mark
	.byte	4                       # Abbrev [4] 0xc47:0x5 DW_TAG_pointer_type
	.long	3148                    # DW_AT_type
	.byte	18                      # Abbrev [18] 0xc4c:0x49 DW_TAG_structure_type
	.long	.Linfo_string218        # DW_AT_name
	.byte	2                       # DW_AT_byte_size
	.byte	2                       # DW_AT_decl_file
	.byte	226                     # DW_AT_decl_line
	.byte	19                      # Abbrev [19] 0xc54:0x10 DW_TAG_member
	.long	.Linfo_string214        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
	.byte	2                       # DW_AT_decl_file
	.byte	228                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	20                      # Abbrev [20] 0xc64:0x10 DW_TAG_member
	.long	.Linfo_string215        # DW_AT_name
	.long	3221                    # DW_AT_type
	.byte	2                       # DW_AT_decl_file
	.byte	228                     # DW_AT_decl_line
	.byte	8                       # DW_AT_byte_size
	.byte	8                       # DW_AT_bit_size
	.byte	55                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	19                      # Abbrev [19] 0xc74:0x10 DW_TAG_member
	.long	.Linfo_string216        # DW_AT_name
	.long	.Lsection_info+766      # DW_AT_type
	.byte	2                       # DW_AT_decl_file
	.byte	228                     # DW_AT_decl_line
	.byte	4                       # DW_AT_byte_size
	.byte	4                       # DW_AT_bit_size
	.byte	27                      # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	19                      # Abbrev [19] 0xc84:0x10 DW_TAG_member
	.long	.Linfo_string217        # DW_AT_name
	.long	.Lsection_info+759      # DW_AT_type
	.byte	2                       # DW_AT_decl_file
	.byte	228                     # DW_AT_decl_line
	.byte	1                       # DW_AT_byte_size
	.byte	1                       # DW_AT_bit_size
	.byte	6                       # DW_AT_bit_offset
	.byte	0                       # DW_AT_data_member_location
	.byte	1                       # DW_AT_accessibility
                                        # DW_ACCESS_public
	.byte	0                       # End Of Children Mark
	.byte	21                      # Abbrev [21] 0xc95:0x5 DW_TAG_pointer_type
	.long	.Lsection_info+759      # DW_AT_type
	.byte	0                       # End Of Children Mark
.L.debug_info_end1:
.L.debug_info_begin2:
	.long	172                     # Length of Unit
	.short	4                       # DWARF version number
	.long	.L.debug_abbrev_begin   # Offset Into Abbrev. Section
	.byte	8                       # Address Size (in bytes)
	.byte	1                       # Abbrev [1] 0xb:0xa5 DW_TAG_compile_unit
	.long	.Linfo_string0          # DW_AT_producer
	.short	0                       # DW_AT_language
	.long	.Linfo_string186        # DW_AT_name
	.quad	0                       # DW_AT_low_pc
	.long	.Lsection_line          # DW_AT_stmt_list
	.long	.Linfo_string2          # DW_AT_comp_dir
	.byte	14                      # Abbrev [14] 0x26:0x19 DW_TAG_variable
	.long	.Linfo_string3          # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	3                       # DW_AT_decl_file
	.byte	3                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	RAND_MAX
	.long	.Linfo_string3          # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x3f:0x19 DW_TAG_variable
	.long	.Linfo_string187        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	3                       # DW_AT_decl_file
	.byte	4                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	EXIT_FAILURE
	.long	.Linfo_string187        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x58:0x19 DW_TAG_variable
	.long	.Linfo_string188        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	3                       # DW_AT_decl_file
	.byte	5                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	EXIT_SUCCESS
	.long	.Linfo_string188        # DW_AT_MIPS_linkage_name
	.byte	16                      # Abbrev [16] 0x71:0x3e DW_TAG_subprogram
	.long	.Linfo_string189        # DW_AT_MIPS_linkage_name
	.long	.Linfo_string189        # DW_AT_name
	.byte	3                       # DW_AT_decl_file
	.byte	56                      # DW_AT_decl_line
	.long	.Lsection_info+1107     # DW_AT_type
                                        # DW_AT_external
	.quad	.Lfunc_begin7           # DW_AT_low_pc
	.quad	.Lfunc_end7             # DW_AT_high_pc
	.byte	1                       # DW_AT_frame_base
	.byte	86
	.byte	17                      # Abbrev [17] 0x92:0xe DW_TAG_formal_parameter
	.long	.Linfo_string220        # DW_AT_name
	.byte	3                       # DW_AT_decl_file
	.byte	56                      # DW_AT_decl_line
	.long	.Lsection_info+1010     # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	120
	.byte	17                      # Abbrev [17] 0xa0:0xe DW_TAG_formal_parameter
	.long	.Linfo_string221        # DW_AT_name
	.byte	3                       # DW_AT_decl_file
	.byte	56                      # DW_AT_decl_line
	.long	.Lsection_info+780      # DW_AT_type
	.byte	2                       # DW_AT_location
	.byte	145
	.byte	112
	.byte	0                       # End Of Children Mark
	.byte	0                       # End Of Children Mark
.L.debug_info_end2:
.L.debug_info_begin3:
	.long	248                     # Length of Unit
	.short	4                       # DWARF version number
	.long	.L.debug_abbrev_begin   # Offset Into Abbrev. Section
	.byte	8                       # Address Size (in bytes)
	.byte	1                       # Abbrev [1] 0xb:0xf1 DW_TAG_compile_unit
	.long	.Linfo_string0          # DW_AT_producer
	.short	0                       # DW_AT_language
	.long	.Linfo_string190        # DW_AT_name
	.quad	0                       # DW_AT_low_pc
	.long	.Lsection_line          # DW_AT_stmt_list
	.long	.Linfo_string2          # DW_AT_comp_dir
	.byte	14                      # Abbrev [14] 0x26:0x19 DW_TAG_variable
	.long	.Linfo_string191        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	4                       # DW_AT_decl_file
	.byte	1                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	EOF
	.long	.Linfo_string191        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x3f:0x19 DW_TAG_variable
	.long	.Linfo_string192        # DW_AT_name
	.long	.Lsection_info+780      # DW_AT_type
                                        # DW_AT_external
	.byte	4                       # DW_AT_decl_file
	.byte	2                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	P_tmpdir
	.long	.Linfo_string192        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x58:0x19 DW_TAG_variable
	.long	.Linfo_string193        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	4                       # DW_AT_decl_file
	.byte	4                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SEEK_SET
	.long	.Linfo_string193        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x71:0x19 DW_TAG_variable
	.long	.Linfo_string194        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	4                       # DW_AT_decl_file
	.byte	5                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SEEK_CUR
	.long	.Linfo_string194        # DW_AT_MIPS_linkage_name
	.byte	14                      # Abbrev [14] 0x8a:0x19 DW_TAG_variable
	.long	.Linfo_string195        # DW_AT_name
	.long	.Lsection_info+63       # DW_AT_type
                                        # DW_AT_external
	.byte	4                       # DW_AT_decl_file
	.byte	6                       # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	SEEK_END
	.long	.Linfo_string195        # DW_AT_MIPS_linkage_name
	.byte	2                       # Abbrev [2] 0xa3:0x19 DW_TAG_variable
	.long	.Linfo_string196        # DW_AT_name
	.long	188                     # DW_AT_type
                                        # DW_AT_external
	.byte	4                       # DW_AT_decl_file
	.byte	10                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	stdin
	.long	.Linfo_string196        # DW_AT_MIPS_linkage_name
	.byte	4                       # Abbrev [4] 0xbc:0x5 DW_TAG_pointer_type
	.long	193                     # DW_AT_type
	.byte	22                      # Abbrev [22] 0xc1:0x8 DW_TAG_structure_type
	.long	.Linfo_string197        # DW_AT_name
	.byte	0                       # DW_AT_byte_size
	.byte	4                       # DW_AT_decl_file
	.byte	9                       # DW_AT_decl_line
	.byte	2                       # Abbrev [2] 0xc9:0x19 DW_TAG_variable
	.long	.Linfo_string198        # DW_AT_name
	.long	188                     # DW_AT_type
                                        # DW_AT_external
	.byte	4                       # DW_AT_decl_file
	.byte	11                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	stdout
	.long	.Linfo_string198        # DW_AT_MIPS_linkage_name
	.byte	2                       # Abbrev [2] 0xe2:0x19 DW_TAG_variable
	.long	.Linfo_string199        # DW_AT_name
	.long	188                     # DW_AT_type
                                        # DW_AT_external
	.byte	4                       # DW_AT_decl_file
	.byte	12                      # DW_AT_decl_line
	.byte	9                       # DW_AT_location
	.byte	3
	.quad	stderr
	.long	.Linfo_string199        # DW_AT_MIPS_linkage_name
	.byte	0                       # End Of Children Mark
.L.debug_info_end3:
	.section	.debug_abbrev,"",@progbits
.L.debug_abbrev_begin:
	.byte	1                       # Abbreviation Code
	.byte	17                      # DW_TAG_compile_unit
	.byte	1                       # DW_CHILDREN_yes
	.byte	37                      # DW_AT_producer
	.byte	14                      # DW_FORM_strp
	.byte	19                      # DW_AT_language
	.byte	5                       # DW_FORM_data2
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	17                      # DW_AT_low_pc
	.byte	1                       # DW_FORM_addr
	.byte	16                      # DW_AT_stmt_list
	.byte	23                      # DW_FORM_sec_offset
	.byte	27                      # DW_AT_comp_dir
	.byte	14                      # DW_FORM_strp
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	2                       # Abbreviation Code
	.byte	52                      # DW_TAG_variable
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	73                      # DW_AT_type
	.byte	19                      # DW_FORM_ref4
	.byte	63                      # DW_AT_external
	.byte	25                      # DW_FORM_flag_present
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	2                       # DW_AT_location
	.byte	10                      # DW_FORM_block1
	.ascii	"\207@"                 # DW_AT_MIPS_linkage_name
	.byte	14                      # DW_FORM_strp
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	3                       # Abbreviation Code
	.byte	36                      # DW_TAG_base_type
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	62                      # DW_AT_encoding
	.byte	11                      # DW_FORM_data1
	.byte	11                      # DW_AT_byte_size
	.byte	11                      # DW_FORM_data1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	4                       # Abbreviation Code
	.byte	15                      # DW_TAG_pointer_type
	.byte	0                       # DW_CHILDREN_no
	.byte	73                      # DW_AT_type
	.byte	19                      # DW_FORM_ref4
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	5                       # Abbreviation Code
	.byte	19                      # DW_TAG_structure_type
	.byte	1                       # DW_CHILDREN_yes
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	11                      # DW_AT_byte_size
	.byte	11                      # DW_FORM_data1
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	5                       # DW_FORM_data2
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	6                       # Abbreviation Code
	.byte	13                      # DW_TAG_member
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	73                      # DW_AT_type
	.byte	19                      # DW_FORM_ref4
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	5                       # DW_FORM_data2
	.byte	11                      # DW_AT_byte_size
	.byte	11                      # DW_FORM_data1
	.byte	13                      # DW_AT_bit_size
	.byte	11                      # DW_FORM_data1
	.byte	12                      # DW_AT_bit_offset
	.byte	11                      # DW_FORM_data1
	.byte	56                      # DW_AT_data_member_location
	.byte	11                      # DW_FORM_data1
	.byte	50                      # DW_AT_accessibility
	.byte	11                      # DW_FORM_data1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	7                       # Abbreviation Code
	.byte	13                      # DW_TAG_member
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	73                      # DW_AT_type
	.byte	19                      # DW_FORM_ref4
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	5                       # DW_FORM_data2
	.byte	11                      # DW_AT_byte_size
	.byte	11                      # DW_FORM_data1
	.byte	13                      # DW_AT_bit_size
	.byte	11                      # DW_FORM_data1
	.byte	12                      # DW_AT_bit_offset
	.byte	11                      # DW_FORM_data1
	.byte	56                      # DW_AT_data_member_location
	.byte	11                      # DW_FORM_data1
	.byte	50                      # DW_AT_accessibility
	.byte	11                      # DW_FORM_data1
	.byte	76                      # DW_AT_virtuality
	.byte	11                      # DW_FORM_data1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	8                       # Abbreviation Code
	.byte	13                      # DW_TAG_member
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	73                      # DW_AT_type
	.byte	19                      # DW_FORM_ref4
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	5                       # DW_FORM_data2
	.byte	56                      # DW_AT_data_member_location
	.byte	11                      # DW_FORM_data1
	.byte	50                      # DW_AT_accessibility
	.byte	11                      # DW_FORM_data1
	.byte	76                      # DW_AT_virtuality
	.byte	11                      # DW_FORM_data1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	9                       # Abbreviation Code
	.byte	13                      # DW_TAG_member
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	73                      # DW_AT_type
	.byte	19                      # DW_FORM_ref4
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	5                       # DW_FORM_data2
	.byte	11                      # DW_AT_byte_size
	.byte	11                      # DW_FORM_data1
	.byte	13                      # DW_AT_bit_size
	.byte	11                      # DW_FORM_data1
	.byte	12                      # DW_AT_bit_offset
	.byte	11                      # DW_FORM_data1
	.byte	56                      # DW_AT_data_member_location
	.byte	11                      # DW_FORM_data1
	.byte	50                      # DW_AT_accessibility
	.byte	11                      # DW_FORM_data1
	.byte	52                      # DW_AT_artificial
	.byte	25                      # DW_FORM_flag_present
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	10                      # Abbreviation Code
	.byte	46                      # DW_TAG_subprogram
	.byte	1                       # DW_CHILDREN_yes
	.ascii	"\207@"                 # DW_AT_MIPS_linkage_name
	.byte	14                      # DW_FORM_strp
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	73                      # DW_AT_type
	.byte	19                      # DW_FORM_ref4
	.byte	63                      # DW_AT_external
	.byte	25                      # DW_FORM_flag_present
	.byte	17                      # DW_AT_low_pc
	.byte	1                       # DW_FORM_addr
	.byte	18                      # DW_AT_high_pc
	.byte	1                       # DW_FORM_addr
	.byte	64                      # DW_AT_frame_base
	.byte	10                      # DW_FORM_block1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	11                      # Abbreviation Code
	.byte	5                       # DW_TAG_formal_parameter
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	73                      # DW_AT_type
	.byte	19                      # DW_FORM_ref4
	.byte	2                       # DW_AT_location
	.byte	10                      # DW_FORM_block1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	12                      # Abbreviation Code
	.byte	52                      # DW_TAG_variable
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	73                      # DW_AT_type
	.byte	19                      # DW_FORM_ref4
	.byte	2                       # DW_AT_location
	.byte	10                      # DW_FORM_block1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	13                      # Abbreviation Code
	.byte	46                      # DW_TAG_subprogram
	.byte	0                       # DW_CHILDREN_no
	.ascii	"\207@"                 # DW_AT_MIPS_linkage_name
	.byte	14                      # DW_FORM_strp
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	73                      # DW_AT_type
	.byte	19                      # DW_FORM_ref4
	.byte	63                      # DW_AT_external
	.byte	25                      # DW_FORM_flag_present
	.byte	17                      # DW_AT_low_pc
	.byte	1                       # DW_FORM_addr
	.byte	18                      # DW_AT_high_pc
	.byte	1                       # DW_FORM_addr
	.byte	64                      # DW_AT_frame_base
	.byte	10                      # DW_FORM_block1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	14                      # Abbreviation Code
	.byte	52                      # DW_TAG_variable
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	73                      # DW_AT_type
	.byte	16                      # DW_FORM_ref_addr
	.byte	63                      # DW_AT_external
	.byte	25                      # DW_FORM_flag_present
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	2                       # DW_AT_location
	.byte	10                      # DW_FORM_block1
	.ascii	"\207@"                 # DW_AT_MIPS_linkage_name
	.byte	14                      # DW_FORM_strp
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	15                      # Abbreviation Code
	.byte	52                      # DW_TAG_variable
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	73                      # DW_AT_type
	.byte	16                      # DW_FORM_ref_addr
	.byte	63                      # DW_AT_external
	.byte	25                      # DW_FORM_flag_present
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	5                       # DW_FORM_data2
	.byte	2                       # DW_AT_location
	.byte	10                      # DW_FORM_block1
	.ascii	"\207@"                 # DW_AT_MIPS_linkage_name
	.byte	14                      # DW_FORM_strp
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	16                      # Abbreviation Code
	.byte	46                      # DW_TAG_subprogram
	.byte	1                       # DW_CHILDREN_yes
	.ascii	"\207@"                 # DW_AT_MIPS_linkage_name
	.byte	14                      # DW_FORM_strp
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	73                      # DW_AT_type
	.byte	16                      # DW_FORM_ref_addr
	.byte	63                      # DW_AT_external
	.byte	25                      # DW_FORM_flag_present
	.byte	17                      # DW_AT_low_pc
	.byte	1                       # DW_FORM_addr
	.byte	18                      # DW_AT_high_pc
	.byte	1                       # DW_FORM_addr
	.byte	64                      # DW_AT_frame_base
	.byte	10                      # DW_FORM_block1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	17                      # Abbreviation Code
	.byte	5                       # DW_TAG_formal_parameter
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	73                      # DW_AT_type
	.byte	16                      # DW_FORM_ref_addr
	.byte	2                       # DW_AT_location
	.byte	10                      # DW_FORM_block1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	18                      # Abbreviation Code
	.byte	19                      # DW_TAG_structure_type
	.byte	1                       # DW_CHILDREN_yes
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	11                      # DW_AT_byte_size
	.byte	11                      # DW_FORM_data1
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	19                      # Abbreviation Code
	.byte	13                      # DW_TAG_member
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	73                      # DW_AT_type
	.byte	16                      # DW_FORM_ref_addr
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	11                      # DW_AT_byte_size
	.byte	11                      # DW_FORM_data1
	.byte	13                      # DW_AT_bit_size
	.byte	11                      # DW_FORM_data1
	.byte	12                      # DW_AT_bit_offset
	.byte	11                      # DW_FORM_data1
	.byte	56                      # DW_AT_data_member_location
	.byte	11                      # DW_FORM_data1
	.byte	50                      # DW_AT_accessibility
	.byte	11                      # DW_FORM_data1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	20                      # Abbreviation Code
	.byte	13                      # DW_TAG_member
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	73                      # DW_AT_type
	.byte	19                      # DW_FORM_ref4
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	11                      # DW_AT_byte_size
	.byte	11                      # DW_FORM_data1
	.byte	13                      # DW_AT_bit_size
	.byte	11                      # DW_FORM_data1
	.byte	12                      # DW_AT_bit_offset
	.byte	11                      # DW_FORM_data1
	.byte	56                      # DW_AT_data_member_location
	.byte	11                      # DW_FORM_data1
	.byte	50                      # DW_AT_accessibility
	.byte	11                      # DW_FORM_data1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	21                      # Abbreviation Code
	.byte	15                      # DW_TAG_pointer_type
	.byte	0                       # DW_CHILDREN_no
	.byte	73                      # DW_AT_type
	.byte	16                      # DW_FORM_ref_addr
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	22                      # Abbreviation Code
	.byte	19                      # DW_TAG_structure_type
	.byte	0                       # DW_CHILDREN_no
	.byte	3                       # DW_AT_name
	.byte	14                      # DW_FORM_strp
	.byte	11                      # DW_AT_byte_size
	.byte	11                      # DW_FORM_data1
	.byte	58                      # DW_AT_decl_file
	.byte	11                      # DW_FORM_data1
	.byte	59                      # DW_AT_decl_line
	.byte	11                      # DW_FORM_data1
	.byte	0                       # EOM(1)
	.byte	0                       # EOM(2)
	.byte	0                       # EOM(3)
.L.debug_abbrev_end:
	.section	.debug_aranges,"",@progbits
	.long	108                     # Length of ARange Set
	.short	2                       # DWARF Arange version number
	.long	.L.debug_info_begin0    # Offset Into Debug Info Section
	.byte	8                       # Address Size (in bytes)
	.byte	0                       # Segment Size (in bytes)
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.quad	back
.Lset0 = .Ldebug_end6-back
	.quad	.Lset0
	.quad	surf
.Lset1 = .Ldebug_end7-surf
	.quad	.Lset1
	.quad	testval
.Lset2 = .Ldebug_end8-testval
	.quad	.Lset2
	.quad	RAND_MAX1
.Lset3 = .Ldebug_end34-RAND_MAX1
	.quad	.Lset3
	.quad	.Lfunc_begin0
.Lset4 = .Lfunc_begin6-.Lfunc_begin0
	.quad	.Lset4
	.quad	0                       # ARange terminator
	.quad	0
	.long	1980                    # Length of ARange Set
	.short	2                       # DWARF Arange version number
	.long	.L.debug_info_begin1    # Offset Into Debug Info Section
	.byte	8                       # Address Size (in bytes)
	.byte	0                       # Segment Size (in bytes)
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.quad	KMOD_NONE
.Lset5 = .Ldebug_end1-KMOD_NONE
	.quad	.Lset5
	.quad	MIX_NO_FADING
.Lset6 = .Ldebug_end2-MIX_NO_FADING
	.quad	.Lset6
	.quad	SDL_AUDIO_STOPPED
.Lset7 = .Ldebug_end3-SDL_AUDIO_STOPPED
	.quad	.Lset7
	.quad	SDL_SWSURFACE
.Lset8 = .Ldebug_end4-SDL_SWSURFACE
	.quad	.Lset8
	.quad	AUDIO_S16LSB
.Lset9 = .Ldebug_end10-AUDIO_S16LSB
	.quad	.Lset9
	.quad	AUDIO_S8
.Lset10 = .Ldebug_end11-AUDIO_S8
	.quad	.Lset10
	.quad	AUDIO_U16LSB
.Lset11 = .Ldebug_end12-AUDIO_U16LSB
	.quad	.Lset11
	.quad	AUDIO_U8
.Lset12 = .Ldebug_end13-AUDIO_U8
	.quad	.Lset12
	.quad	IMG_INIT_JPG
.Lset13 = .Ldebug_end16-IMG_INIT_JPG
	.quad	.Lset13
	.quad	IMG_INIT_PNG
.Lset14 = .Ldebug_end17-IMG_INIT_PNG
	.quad	.Lset14
	.quad	IMG_INIT_TIF
.Lset15 = .Ldebug_end18-IMG_INIT_TIF
	.quad	.Lset15
	.quad	IMG_INIT_WEBP
.Lset16 = .Ldebug_end19-IMG_INIT_WEBP
	.quad	.Lset16
	.quad	KMOD_LALT
.Lset17 = .Ldebug_end20-KMOD_LALT
	.quad	.Lset17
	.quad	KMOD_LCTRL
.Lset18 = .Ldebug_end21-KMOD_LCTRL
	.quad	.Lset18
	.quad	KMOD_LSHIFT
.Lset19 = .Ldebug_end22-KMOD_LSHIFT
	.quad	.Lset19
	.quad	KMOD_RALT
.Lset20 = .Ldebug_end23-KMOD_RALT
	.quad	.Lset20
	.quad	KMOD_RCTRL
.Lset21 = .Ldebug_end24-KMOD_RCTRL
	.quad	.Lset21
	.quad	KMOD_RSHIFT
.Lset22 = .Ldebug_end25-KMOD_RSHIFT
	.quad	.Lset22
	.quad	MIX_CHANNELS
.Lset23 = .Ldebug_end26-MIX_CHANNELS
	.quad	.Lset23
	.quad	MIX_FADING_IN
.Lset24 = .Ldebug_end27-MIX_FADING_IN
	.quad	.Lset24
	.quad	MIX_FADING_OUT
.Lset25 = .Ldebug_end28-MIX_FADING_OUT
	.quad	.Lset25
	.quad	MIX_INIT_FLAC
.Lset26 = .Ldebug_end29-MIX_INIT_FLAC
	.quad	.Lset26
	.quad	MIX_INIT_MOD
.Lset27 = .Ldebug_end30-MIX_INIT_MOD
	.quad	.Lset27
	.quad	MIX_INIT_MP3
.Lset28 = .Ldebug_end31-MIX_INIT_MP3
	.quad	.Lset28
	.quad	MIX_INIT_OGG
.Lset29 = .Ldebug_end32-MIX_INIT_OGG
	.quad	.Lset29
	.quad	SDLK_0
.Lset30 = .Ldebug_end36-SDLK_0
	.quad	.Lset30
	.quad	SDLK_1
.Lset31 = .Ldebug_end37-SDLK_1
	.quad	.Lset31
	.quad	SDLK_2
.Lset32 = .Ldebug_end38-SDLK_2
	.quad	.Lset32
	.quad	SDLK_3
.Lset33 = .Ldebug_end39-SDLK_3
	.quad	.Lset33
	.quad	SDLK_4
.Lset34 = .Ldebug_end40-SDLK_4
	.quad	.Lset34
	.quad	SDLK_5
.Lset35 = .Ldebug_end41-SDLK_5
	.quad	.Lset35
	.quad	SDLK_6
.Lset36 = .Ldebug_end42-SDLK_6
	.quad	.Lset36
	.quad	SDLK_7
.Lset37 = .Ldebug_end43-SDLK_7
	.quad	.Lset37
	.quad	SDLK_8
.Lset38 = .Ldebug_end44-SDLK_8
	.quad	.Lset38
	.quad	SDLK_9
.Lset39 = .Ldebug_end45-SDLK_9
	.quad	.Lset39
	.quad	SDLK_AMPERSAND
.Lset40 = .Ldebug_end46-SDLK_AMPERSAND
	.quad	.Lset40
	.quad	SDLK_ASTERISK
.Lset41 = .Ldebug_end47-SDLK_ASTERISK
	.quad	.Lset41
	.quad	SDLK_AT
.Lset42 = .Ldebug_end48-SDLK_AT
	.quad	.Lset42
	.quad	SDLK_BACKQUOTE
.Lset43 = .Ldebug_end49-SDLK_BACKQUOTE
	.quad	.Lset43
	.quad	SDLK_BACKSLASH
.Lset44 = .Ldebug_end50-SDLK_BACKSLASH
	.quad	.Lset44
	.quad	SDLK_BACKSPACE
.Lset45 = .Ldebug_end51-SDLK_BACKSPACE
	.quad	.Lset45
	.quad	SDLK_CARET
.Lset46 = .Ldebug_end52-SDLK_CARET
	.quad	.Lset46
	.quad	SDLK_CLEAR
.Lset47 = .Ldebug_end53-SDLK_CLEAR
	.quad	.Lset47
	.quad	SDLK_COLON
.Lset48 = .Ldebug_end54-SDLK_COLON
	.quad	.Lset48
	.quad	SDLK_COMMA
.Lset49 = .Ldebug_end55-SDLK_COMMA
	.quad	.Lset49
	.quad	SDLK_DELETE
.Lset50 = .Ldebug_end56-SDLK_DELETE
	.quad	.Lset50
	.quad	SDLK_DOLLAR
.Lset51 = .Ldebug_end57-SDLK_DOLLAR
	.quad	.Lset51
	.quad	SDLK_DOWN
.Lset52 = .Ldebug_end58-SDLK_DOWN
	.quad	.Lset52
	.quad	SDLK_END
.Lset53 = .Ldebug_end59-SDLK_END
	.quad	.Lset53
	.quad	SDLK_EQUALS
.Lset54 = .Ldebug_end60-SDLK_EQUALS
	.quad	.Lset54
	.quad	SDLK_ESCAPE
.Lset55 = .Ldebug_end61-SDLK_ESCAPE
	.quad	.Lset55
	.quad	SDLK_EXCLAIM
.Lset56 = .Ldebug_end62-SDLK_EXCLAIM
	.quad	.Lset56
	.quad	SDLK_GREATER
.Lset57 = .Ldebug_end63-SDLK_GREATER
	.quad	.Lset57
	.quad	SDLK_HASH
.Lset58 = .Ldebug_end64-SDLK_HASH
	.quad	.Lset58
	.quad	SDLK_HOME
.Lset59 = .Ldebug_end65-SDLK_HOME
	.quad	.Lset59
	.quad	SDLK_INSERT
.Lset60 = .Ldebug_end66-SDLK_INSERT
	.quad	.Lset60
	.quad	SDLK_LEFTBRACKET
.Lset61 = .Ldebug_end67-SDLK_LEFTBRACKET
	.quad	.Lset61
	.quad	SDLK_LEFTPAREN
.Lset62 = .Ldebug_end68-SDLK_LEFTPAREN
	.quad	.Lset62
	.quad	SDLK_LEFT
.Lset63 = .Ldebug_end69-SDLK_LEFT
	.quad	.Lset63
	.quad	SDLK_LESS
.Lset64 = .Ldebug_end70-SDLK_LESS
	.quad	.Lset64
	.quad	SDLK_MINUS
.Lset65 = .Ldebug_end71-SDLK_MINUS
	.quad	.Lset65
	.quad	SDLK_PAGEDOWN
.Lset66 = .Ldebug_end72-SDLK_PAGEDOWN
	.quad	.Lset66
	.quad	SDLK_PAGEUP
.Lset67 = .Ldebug_end73-SDLK_PAGEUP
	.quad	.Lset67
	.quad	SDLK_PAUSE
.Lset68 = .Ldebug_end74-SDLK_PAUSE
	.quad	.Lset68
	.quad	SDLK_PERIOD
.Lset69 = .Ldebug_end75-SDLK_PERIOD
	.quad	.Lset69
	.quad	SDLK_PLUS
.Lset70 = .Ldebug_end76-SDLK_PLUS
	.quad	.Lset70
	.quad	SDLK_QUESTION
.Lset71 = .Ldebug_end77-SDLK_QUESTION
	.quad	.Lset71
	.quad	SDLK_QUOTEDBL
.Lset72 = .Ldebug_end78-SDLK_QUOTEDBL
	.quad	.Lset72
	.quad	SDLK_QUOTE
.Lset73 = .Ldebug_end79-SDLK_QUOTE
	.quad	.Lset73
	.quad	SDLK_RETURN
.Lset74 = .Ldebug_end80-SDLK_RETURN
	.quad	.Lset74
	.quad	SDLK_RIGHTBRACKET
.Lset75 = .Ldebug_end81-SDLK_RIGHTBRACKET
	.quad	.Lset75
	.quad	SDLK_RIGHTPAREN
.Lset76 = .Ldebug_end82-SDLK_RIGHTPAREN
	.quad	.Lset76
	.quad	SDLK_RIGHT
.Lset77 = .Ldebug_end83-SDLK_RIGHT
	.quad	.Lset77
	.quad	SDLK_SEMICOLON
.Lset78 = .Ldebug_end84-SDLK_SEMICOLON
	.quad	.Lset78
	.quad	SDLK_SLASH
.Lset79 = .Ldebug_end85-SDLK_SLASH
	.quad	.Lset79
	.quad	SDLK_SPACE
.Lset80 = .Ldebug_end86-SDLK_SPACE
	.quad	.Lset80
	.quad	SDLK_TAB
.Lset81 = .Ldebug_end87-SDLK_TAB
	.quad	.Lset81
	.quad	SDLK_UNDERSCORE
.Lset82 = .Ldebug_end88-SDLK_UNDERSCORE
	.quad	.Lset82
	.quad	SDLK_UP
.Lset83 = .Ldebug_end89-SDLK_UP
	.quad	.Lset83
	.quad	SDLK_a
.Lset84 = .Ldebug_end90-SDLK_a
	.quad	.Lset84
	.quad	SDLK_b
.Lset85 = .Ldebug_end91-SDLK_b
	.quad	.Lset85
	.quad	SDLK_c
.Lset86 = .Ldebug_end92-SDLK_c
	.quad	.Lset86
	.quad	SDLK_d
.Lset87 = .Ldebug_end93-SDLK_d
	.quad	.Lset87
	.quad	SDLK_e
.Lset88 = .Ldebug_end94-SDLK_e
	.quad	.Lset88
	.quad	SDLK_f
.Lset89 = .Ldebug_end95-SDLK_f
	.quad	.Lset89
	.quad	SDLK_g
.Lset90 = .Ldebug_end96-SDLK_g
	.quad	.Lset90
	.quad	SDLK_h
.Lset91 = .Ldebug_end97-SDLK_h
	.quad	.Lset91
	.quad	SDLK_i
.Lset92 = .Ldebug_end98-SDLK_i
	.quad	.Lset92
	.quad	SDLK_j
.Lset93 = .Ldebug_end99-SDLK_j
	.quad	.Lset93
	.quad	SDLK_k
.Lset94 = .Ldebug_end100-SDLK_k
	.quad	.Lset94
	.quad	SDLK_l
.Lset95 = .Ldebug_end101-SDLK_l
	.quad	.Lset95
	.quad	SDLK_m
.Lset96 = .Ldebug_end102-SDLK_m
	.quad	.Lset96
	.quad	SDLK_n
.Lset97 = .Ldebug_end103-SDLK_n
	.quad	.Lset97
	.quad	SDLK_o
.Lset98 = .Ldebug_end104-SDLK_o
	.quad	.Lset98
	.quad	SDLK_p
.Lset99 = .Ldebug_end105-SDLK_p
	.quad	.Lset99
	.quad	SDLK_q
.Lset100 = .Ldebug_end106-SDLK_q
	.quad	.Lset100
	.quad	SDLK_r
.Lset101 = .Ldebug_end107-SDLK_r
	.quad	.Lset101
	.quad	SDLK_s
.Lset102 = .Ldebug_end108-SDLK_s
	.quad	.Lset102
	.quad	SDLK_t
.Lset103 = .Ldebug_end109-SDLK_t
	.quad	.Lset103
	.quad	SDLK_u
.Lset104 = .Ldebug_end110-SDLK_u
	.quad	.Lset104
	.quad	SDLK_v
.Lset105 = .Ldebug_end111-SDLK_v
	.quad	.Lset105
	.quad	SDLK_w
.Lset106 = .Ldebug_end112-SDLK_w
	.quad	.Lset106
	.quad	SDLK_x
.Lset107 = .Ldebug_end113-SDLK_x
	.quad	.Lset107
	.quad	SDLK_y
.Lset108 = .Ldebug_end114-SDLK_y
	.quad	.Lset108
	.quad	SDLK_z
.Lset109 = .Ldebug_end115-SDLK_z
	.quad	.Lset109
	.quad	SDL_APPACTIVE
.Lset110 = .Ldebug_end116-SDL_APPACTIVE
	.quad	.Lset110
	.quad	SDL_APPINPUTFOCUS
.Lset111 = .Ldebug_end117-SDL_APPINPUTFOCUS
	.quad	.Lset111
	.quad	SDL_APPMOUSEFOCUS
.Lset112 = .Ldebug_end118-SDL_APPMOUSEFOCUS
	.quad	.Lset112
	.quad	SDL_AUDIO_PAUSED
.Lset113 = .Ldebug_end119-SDL_AUDIO_PAUSED
	.quad	.Lset113
	.quad	SDL_AUDIO_PLAYING
.Lset114 = .Ldebug_end120-SDL_AUDIO_PLAYING
	.quad	.Lset114
	.quad	SDL_BUTTON_LEFT
.Lset115 = .Ldebug_end121-SDL_BUTTON_LEFT
	.quad	.Lset115
	.quad	SDL_BUTTON_MIDDLE
.Lset116 = .Ldebug_end122-SDL_BUTTON_MIDDLE
	.quad	.Lset116
	.quad	SDL_BUTTON_RIGHT
.Lset117 = .Ldebug_end123-SDL_BUTTON_RIGHT
	.quad	.Lset117
	.quad	SDL_DEFAULT_REPEAT_DELAY
.Lset118 = .Ldebug_end124-SDL_DEFAULT_REPEAT_DELAY
	.quad	.Lset118
	.quad	SDL_DEFAULT_REPEAT_INTERVAL
.Lset119 = .Ldebug_end125-SDL_DEFAULT_REPEAT_INTERVAL
	.quad	.Lset119
	.quad	SDL_INIT_AUDIO
.Lset120 = .Ldebug_end126-SDL_INIT_AUDIO
	.quad	.Lset120
	.quad	SDL_INIT_CDROM
.Lset121 = .Ldebug_end127-SDL_INIT_CDROM
	.quad	.Lset121
	.quad	SDL_INIT_EVERYTHING
.Lset122 = .Ldebug_end128-SDL_INIT_EVERYTHING
	.quad	.Lset122
	.quad	SDL_INIT_JOYSTICK
.Lset123 = .Ldebug_end129-SDL_INIT_JOYSTICK
	.quad	.Lset123
	.quad	SDL_INIT_TIMER
.Lset124 = .Ldebug_end130-SDL_INIT_TIMER
	.quad	.Lset124
	.quad	SDL_INIT_VIDEO
.Lset125 = .Ldebug_end131-SDL_INIT_VIDEO
	.quad	.Lset125
	.quad	.Lfunc_begin6
.Lset126 = .Lfunc_begin7-.Lfunc_begin6
	.quad	.Lset126
	.quad	0                       # ARange terminator
	.quad	0
	.long	92                      # Length of ARange Set
	.short	2                       # DWARF Arange version number
	.long	.L.debug_info_begin2    # Offset Into Debug Info Section
	.byte	8                       # Address Size (in bytes)
	.byte	0                       # Segment Size (in bytes)
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.quad	EXIT_SUCCESS
.Lset127 = .Ldebug_end0-EXIT_SUCCESS
	.quad	.Lset127
	.quad	EXIT_FAILURE
.Lset128 = .Ldebug_end15-EXIT_FAILURE
	.quad	.Lset128
	.quad	RAND_MAX
.Lset129 = .Ldebug_end35-RAND_MAX
	.quad	.Lset129
	.quad	.Lfunc_begin7
.Lset130 = .Ldebug_end134-.Lfunc_begin7
	.quad	.Lset130
	.quad	0                       # ARange terminator
	.quad	0
	.long	124                     # Length of ARange Set
	.short	2                       # DWARF Arange version number
	.long	.L.debug_info_begin3    # Offset Into Debug Info Section
	.byte	8                       # Address Size (in bytes)
	.byte	0                       # Segment Size (in bytes)
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.quad	SEEK_SET
.Lset131 = .Ldebug_end5-SEEK_SET
	.quad	.Lset131
	.quad	stdin
.Lset132 = .Ldebug_end9-stdin
	.quad	.Lset132
	.quad	EOF
.Lset133 = .Ldebug_end14-EOF
	.quad	.Lset133
	.quad	P_tmpdir
.Lset134 = .Ldebug_end33-P_tmpdir
	.quad	.Lset134
	.quad	SEEK_CUR
.Lset135 = .Ldebug_end132-SEEK_CUR
	.quad	.Lset135
	.quad	SEEK_END
.Lset136 = .Ldebug_end133-SEEK_END
	.quad	.Lset136
	.quad	0                       # ARange terminator
	.quad	0
	.section	.debug_ranges,"",@progbits
	.section	.debug_macinfo,"",@progbits
	.section	.debug_pubnames,"",@progbits
.Lset137 = .Lpubnames_end0-.Lpubnames_begin0 # Length of Public Names Info
	.long	.Lset137
.Lpubnames_begin0:
	.short	2                       # DWARF Version
	.long	.L.debug_info_begin0    # Offset of Compilation Unit Info
.Lset138 = .L.debug_info_end0-.L.debug_info_begin0 # Compilation Unit Length
	.long	.Lset138
	.long	70                      # DIE offset
	.asciz	"surf"                  # External Name
	.long	870                     # DIE offset
	.asciz	"back"                  # External Name
	.long	895                     # DIE offset
	.asciz	"testval"               # External Name
	.long	1114                    # DIE offset
	.asciz	"update"                # External Name
	.long	1271                    # DIE offset
	.asciz	"main"                  # External Name
	.long	1238                    # DIE offset
	.asciz	"somefunc"              # External Name
	.long	1176                    # DIE offset
	.asciz	"randomize"             # External Name
	.long	38                      # DIE offset
	.asciz	"RAND_MAX"              # External Name
	.long	1017                    # DIE offset
	.asciz	"setPixel"              # External Name
	.long	920                     # DIE offset
	.asciz	"pixelIsWhite"          # External Name
	.long	0                       # End Mark
.Lpubnames_end0:
.Lset139 = .Lpubnames_end2-.Lpubnames_begin2 # Length of Public Names Info
	.long	.Lset139
.Lpubnames_begin2:
	.short	2                       # DWARF Version
	.long	.L.debug_info_begin2    # Offset of Compilation Unit Info
.Lset140 = .L.debug_info_end2-.L.debug_info_begin2 # Compilation Unit Length
	.long	.Lset140
	.long	63                      # DIE offset
	.asciz	"EXIT_FAILURE"          # External Name
	.long	88                      # DIE offset
	.asciz	"EXIT_SUCCESS"          # External Name
	.long	113                     # DIE offset
	.asciz	"assert"                # External Name
	.long	38                      # DIE offset
	.asciz	"RAND_MAX"              # External Name
	.long	0                       # End Mark
.Lpubnames_end2:
.Lset141 = .Lpubnames_end3-.Lpubnames_begin3 # Length of Public Names Info
	.long	.Lset141
.Lpubnames_begin3:
	.short	2                       # DWARF Version
	.long	.L.debug_info_begin3    # Offset of Compilation Unit Info
.Lset142 = .L.debug_info_end3-.L.debug_info_begin3 # Compilation Unit Length
	.long	.Lset142
	.long	113                     # DIE offset
	.asciz	"SEEK_CUR"              # External Name
	.long	163                     # DIE offset
	.asciz	"stdin"                 # External Name
	.long	88                      # DIE offset
	.asciz	"SEEK_SET"              # External Name
	.long	201                     # DIE offset
	.asciz	"stdout"                # External Name
	.long	226                     # DIE offset
	.asciz	"stderr"                # External Name
	.long	38                      # DIE offset
	.asciz	"EOF"                   # External Name
	.long	138                     # DIE offset
	.asciz	"SEEK_END"              # External Name
	.long	63                      # DIE offset
	.asciz	"P_tmpdir"              # External Name
	.long	0                       # End Mark
.Lpubnames_end3:
.Lset143 = .Lpubnames_end1-.Lpubnames_begin1 # Length of Public Names Info
	.long	.Lset143
.Lpubnames_begin1:
	.short	2                       # DWARF Version
	.long	.L.debug_info_begin1    # Offset of Compilation Unit Info
.Lset144 = .L.debug_info_end1-.L.debug_info_begin1 # Compilation Unit Length
	.long	.Lset144
	.long	2138                    # DIE offset
	.asciz	"SDLK_s"                # External Name
	.long	2163                    # DIE offset
	.asciz	"SDLK_t"                # External Name
	.long	2188                    # DIE offset
	.asciz	"SDLK_u"                # External Name
	.long	2213                    # DIE offset
	.asciz	"SDLK_v"                # External Name
	.long	2238                    # DIE offset
	.asciz	"SDLK_w"                # External Name
	.long	2263                    # DIE offset
	.asciz	"SDLK_x"                # External Name
	.long	2288                    # DIE offset
	.asciz	"SDLK_y"                # External Name
	.long	388                     # DIE offset
	.asciz	"SDL_AUDIO_PLAYING"     # External Name
	.long	1588                    # DIE offset
	.asciz	"SDLK_RIGHTBRACKET"     # External Name
	.long	2913                    # DIE offset
	.asciz	"MIX_FADING_OUT"        # External Name
	.long	2313                    # DIE offset
	.asciz	"SDLK_z"                # External Name
	.long	838                     # DIE offset
	.asciz	"SDLK_DOLLAR"           # External Name
	.long	2563                    # DIE offset
	.asciz	"SDLK_PAGEDOWN"         # External Name
	.long	263                     # DIE offset
	.asciz	"AUDIO_U8"              # External Name
	.long	913                     # DIE offset
	.asciz	"SDLK_LEFTPAREN"        # External Name
	.long	1813                    # DIE offset
	.asciz	"SDLK_f"                # External Name
	.long	1863                    # DIE offset
	.asciz	"SDLK_h"                # External Name
	.long	2488                    # DIE offset
	.asciz	"SDLK_HOME"             # External Name
	.long	1963                    # DIE offset
	.asciz	"SDLK_l"                # External Name
	.long	38                      # DIE offset
	.asciz	"SDL_INIT_TIMER"        # External Name
	.long	2763                    # DIE offset
	.asciz	"MIX_INIT_FLAC"         # External Name
	.long	2438                    # DIE offset
	.asciz	"SDLK_LEFT"             # External Name
	.long	2588                    # DIE offset
	.asciz	"KMOD_NONE"             # External Name
	.long	513                     # DIE offset
	.asciz	"IMG_INIT_WEBP"         # External Name
	.long	2838                    # DIE offset
	.asciz	"MIX_INIT_OGG"          # External Name
	.long	1788                    # DIE offset
	.asciz	"SDLK_e"                # External Name
	.long	2863                    # DIE offset
	.asciz	"MIX_CHANNELS"          # External Name
	.long	1088                    # DIE offset
	.asciz	"SDLK_SLASH"            # External Name
	.long	113                     # DIE offset
	.asciz	"SDL_INIT_CDROM"        # External Name
	.long	588                     # DIE offset
	.asciz	"SDLK_BACKSPACE"        # External Name
	.long	313                     # DIE offset
	.asciz	"AUDIO_U16LSB"          # External Name
	.long	813                     # DIE offset
	.asciz	"SDLK_HASH"             # External Name
	.long	3041                    # DIE offset
	.asciz	"SDL_SWSURFACE"         # External Name
	.long	2638                    # DIE offset
	.asciz	"KMOD_RSHIFT"           # External Name
	.long	888                     # DIE offset
	.asciz	"SDLK_QUOTE"            # External Name
	.long	3015                    # DIE offset
	.asciz	"SDL_BUTTON_RIGHT"      # External Name
	.long	1488                    # DIE offset
	.asciz	"SDLK_QUESTION"         # External Name
	.long	63                      # DIE offset
	.asciz	"SDL_INIT_AUDIO"        # External Name
	.long	2663                    # DIE offset
	.asciz	"KMOD_LCTRL"            # External Name
	.long	663                     # DIE offset
	.asciz	"SDLK_RETURN"           # External Name
	.long	438                     # DIE offset
	.asciz	"IMG_INIT_JPG"          # External Name
	.long	988                     # DIE offset
	.asciz	"SDLK_PLUS"             # External Name
	.long	2688                    # DIE offset
	.asciz	"KMOD_RCTRL"            # External Name
	.long	1463                    # DIE offset
	.asciz	"SDLK_GREATER"          # External Name
	.long	3067                    # DIE offset
	.asciz	"Mix_PlayChannel"       # External Name
	.long	713                     # DIE offset
	.asciz	"SDLK_ESCAPE"           # External Name
	.long	238                     # DIE offset
	.asciz	"SDL_APPACTIVE"         # External Name
	.long	788                     # DIE offset
	.asciz	"SDLK_QUOTEDBL"         # External Name
	.long	2538                    # DIE offset
	.asciz	"SDLK_PAGEUP"           # External Name
	.long	763                     # DIE offset
	.asciz	"SDLK_EXCLAIM"          # External Name
	.long	2363                    # DIE offset
	.asciz	"SDLK_UP"               # External Name
	.long	2613                    # DIE offset
	.asciz	"KMOD_LSHIFT"           # External Name
	.long	1013                    # DIE offset
	.asciz	"SDLK_COMMA"            # External Name
	.long	1613                    # DIE offset
	.asciz	"SDLK_CARET"            # External Name
	.long	938                     # DIE offset
	.asciz	"SDLK_RIGHTPAREN"       # External Name
	.long	1363                    # DIE offset
	.asciz	"SDLK_COLON"            # External Name
	.long	688                     # DIE offset
	.asciz	"SDLK_PAUSE"            # External Name
	.long	2989                    # DIE offset
	.asciz	"SDL_BUTTON_MIDDLE"     # External Name
	.long	363                     # DIE offset
	.asciz	"SDL_AUDIO_STOPPED"     # External Name
	.long	463                     # DIE offset
	.asciz	"IMG_INIT_PNG"          # External Name
	.long	2713                    # DIE offset
	.asciz	"KMOD_LALT"             # External Name
	.long	738                     # DIE offset
	.asciz	"SDLK_SPACE"            # External Name
	.long	163                     # DIE offset
	.asciz	"SDL_INIT_EVERYTHING"   # External Name
	.long	1038                    # DIE offset
	.asciz	"SDLK_MINUS"            # External Name
	.long	2338                    # DIE offset
	.asciz	"SDLK_DELETE"           # External Name
	.long	2788                    # DIE offset
	.asciz	"MIX_INIT_MOD"          # External Name
	.long	2513                    # DIE offset
	.asciz	"SDLK_END"              # External Name
	.long	2963                    # DIE offset
	.asciz	"SDL_BUTTON_LEFT"       # External Name
	.long	2413                    # DIE offset
	.asciz	"SDLK_RIGHT"            # External Name
	.long	2938                    # DIE offset
	.asciz	"MIX_FADING_IN"         # External Name
	.long	2813                    # DIE offset
	.asciz	"MIX_INIT_MP3"          # External Name
	.long	413                     # DIE offset
	.asciz	"SDL_AUDIO_PAUSED"      # External Name
	.long	1438                    # DIE offset
	.asciz	"SDLK_EQUALS"           # External Name
	.long	1563                    # DIE offset
	.asciz	"SDLK_BACKSLASH"        # External Name
	.long	288                     # DIE offset
	.asciz	"AUDIO_S8"              # External Name
	.long	1113                    # DIE offset
	.asciz	"SDLK_0"                # External Name
	.long	1138                    # DIE offset
	.asciz	"SDLK_1"                # External Name
	.long	1163                    # DIE offset
	.asciz	"SDLK_2"                # External Name
	.long	1188                    # DIE offset
	.asciz	"SDLK_3"                # External Name
	.long	1213                    # DIE offset
	.asciz	"SDLK_4"                # External Name
	.long	1238                    # DIE offset
	.asciz	"SDLK_5"                # External Name
	.long	1263                    # DIE offset
	.asciz	"SDLK_6"                # External Name
	.long	613                     # DIE offset
	.asciz	"SDLK_TAB"              # External Name
	.long	1288                    # DIE offset
	.asciz	"SDLK_7"                # External Name
	.long	1313                    # DIE offset
	.asciz	"SDLK_8"                # External Name
	.long	1338                    # DIE offset
	.asciz	"SDLK_9"                # External Name
	.long	863                     # DIE offset
	.asciz	"SDLK_AMPERSAND"        # External Name
	.long	1413                    # DIE offset
	.asciz	"SDLK_LESS"             # External Name
	.long	2388                    # DIE offset
	.asciz	"SDLK_DOWN"             # External Name
	.long	88                      # DIE offset
	.asciz	"SDL_INIT_VIDEO"        # External Name
	.long	1663                    # DIE offset
	.asciz	"SDLK_BACKQUOTE"        # External Name
	.long	1063                    # DIE offset
	.asciz	"SDLK_PERIOD"           # External Name
	.long	2463                    # DIE offset
	.asciz	"SDLK_INSERT"           # External Name
	.long	2888                    # DIE offset
	.asciz	"MIX_NO_FADING"         # External Name
	.long	963                     # DIE offset
	.asciz	"SDLK_ASTERISK"         # External Name
	.long	638                     # DIE offset
	.asciz	"SDLK_CLEAR"            # External Name
	.long	1538                    # DIE offset
	.asciz	"SDLK_LEFTBRACKET"      # External Name
	.long	2738                    # DIE offset
	.asciz	"KMOD_RALT"             # External Name
	.long	1513                    # DIE offset
	.asciz	"SDLK_AT"               # External Name
	.long	138                     # DIE offset
	.asciz	"SDL_INIT_JOYSTICK"     # External Name
	.long	1638                    # DIE offset
	.asciz	"SDLK_UNDERSCORE"       # External Name
	.long	563                     # DIE offset
	.asciz	"SDL_DEFAULT_REPEAT_INTERVAL" # External Name
	.long	338                     # DIE offset
	.asciz	"AUDIO_S16LSB"          # External Name
	.long	188                     # DIE offset
	.asciz	"SDL_APPMOUSEFOCUS"     # External Name
	.long	1688                    # DIE offset
	.asciz	"SDLK_a"                # External Name
	.long	1713                    # DIE offset
	.asciz	"SDLK_b"                # External Name
	.long	1738                    # DIE offset
	.asciz	"SDLK_c"                # External Name
	.long	1763                    # DIE offset
	.asciz	"SDLK_d"                # External Name
	.long	488                     # DIE offset
	.asciz	"IMG_INIT_TIF"          # External Name
	.long	213                     # DIE offset
	.asciz	"SDL_APPINPUTFOCUS"     # External Name
	.long	1838                    # DIE offset
	.asciz	"SDLK_g"                # External Name
	.long	538                     # DIE offset
	.asciz	"SDL_DEFAULT_REPEAT_DELAY" # External Name
	.long	1888                    # DIE offset
	.asciz	"SDLK_i"                # External Name
	.long	1913                    # DIE offset
	.asciz	"SDLK_j"                # External Name
	.long	1938                    # DIE offset
	.asciz	"SDLK_k"                # External Name
	.long	1388                    # DIE offset
	.asciz	"SDLK_SEMICOLON"        # External Name
	.long	1988                    # DIE offset
	.asciz	"SDLK_m"                # External Name
	.long	2013                    # DIE offset
	.asciz	"SDLK_n"                # External Name
	.long	2038                    # DIE offset
	.asciz	"SDLK_o"                # External Name
	.long	2063                    # DIE offset
	.asciz	"SDLK_p"                # External Name
	.long	2088                    # DIE offset
	.asciz	"SDLK_q"                # External Name
	.long	2113                    # DIE offset
	.asciz	"SDLK_r"                # External Name
	.long	0                       # End Mark
.Lpubnames_end1:
	.section	.debug_pubtypes,"",@progbits
.Lset145 = .Lpubtypes_end0-.Lpubtypes_begin0 # Length of Public Types Info
	.long	.Lset145
.Lpubtypes_begin0:
	.short	2                       # DWARF Version
	.long	.L.debug_info_begin0    # Offset of Compilation Unit Info
.Lset146 = .L.debug_info_end0-.L.debug_info_begin0 # Compilation Unit Length
	.long	.Lset146
	.long	674                     # DIE offset
	.asciz	"SDL_Color"             # External Name
	.long	766                     # DIE offset
	.asciz	"uint32"                # External Name
	.long	863                     # DIE offset
	.asciz	"uint16"                # External Name
	.long	100                     # DIE offset
	.asciz	"SDL_Surface"           # External Name
	.long	1010                    # DIE offset
	.asciz	"bool"                  # External Name
	.long	355                     # DIE offset
	.asciz	"SDL_PixelFormat"       # External Name
	.long	63                      # DIE offset
	.asciz	"int32"                 # External Name
	.long	625                     # DIE offset
	.asciz	"SDL_Palette"           # External Name
	.long	773                     # DIE offset
	.asciz	"int16"                 # External Name
	.long	759                     # DIE offset
	.asciz	"uchar"                 # External Name
	.long	1107                    # DIE offset
	.asciz	"void"                  # External Name
	.long	785                     # DIE offset
	.asciz	"SDL_Rect"              # External Name
	.long	1394                    # DIE offset
	.asciz	"float32"               # External Name
	.long	752                     # DIE offset
	.asciz	"char"                  # External Name
	.long	0                       # End Mark
.Lpubtypes_end0:
.Lset147 = .Lpubtypes_end2-.Lpubtypes_begin2 # Length of Public Types Info
	.long	.Lset147
.Lpubtypes_begin2:
	.short	2                       # DWARF Version
	.long	.L.debug_info_begin2    # Offset of Compilation Unit Info
.Lset148 = .L.debug_info_end2-.L.debug_info_begin2 # Compilation Unit Length
	.long	.Lset148
	.long	63                      # DIE offset
	.asciz	"int32"                 # External Name
	.long	1107                    # DIE offset
	.asciz	"void"                  # External Name
	.long	1010                    # DIE offset
	.asciz	"bool"                  # External Name
	.long	0                       # End Mark
.Lpubtypes_end2:
.Lset149 = .Lpubtypes_end3-.Lpubtypes_begin3 # Length of Public Types Info
	.long	.Lset149
.Lpubtypes_begin3:
	.short	2                       # DWARF Version
	.long	.L.debug_info_begin3    # Offset of Compilation Unit Info
.Lset150 = .L.debug_info_end3-.L.debug_info_begin3 # Compilation Unit Length
	.long	.Lset150
	.long	63                      # DIE offset
	.asciz	"int32"                 # External Name
	.long	193                     # DIE offset
	.asciz	"_IO_FILE"              # External Name
	.long	0                       # End Mark
.Lpubtypes_end3:
.Lset151 = .Lpubtypes_end1-.Lpubtypes_begin1 # Length of Public Types Info
	.long	.Lset151
.Lpubtypes_begin1:
	.short	2                       # DWARF Version
	.long	.L.debug_info_begin1    # Offset of Compilation Unit Info
.Lset152 = .L.debug_info_end1-.L.debug_info_begin1 # Compilation Unit Length
	.long	.Lset152
	.long	63                      # DIE offset
	.asciz	"int32"                 # External Name
	.long	759                     # DIE offset
	.asciz	"uchar"                 # External Name
	.long	766                     # DIE offset
	.asciz	"uint32"                # External Name
	.long	3148                    # DIE offset
	.asciz	"Mix_Chunk"             # External Name
	.long	0                       # End Mark
.Lpubtypes_end1:
	.weak	memcpy
	.weak	SDL_Init
	.weak	SDL_InitSubSystem
	.weak	SDL_QuitSubSystem
	.weak	SDL_WasInit
	.weak	SDL_Quit
	.weak	SDL_GetAppState
	.weak	SDL_AudioInit
	.weak	SDL_AudioQuit
	.weak	SDL_AudioDriverName
	.weak	SDL_OpenAudio
	.weak	SDL_GetAudioStatus
	.weak	SDL_PauseAudio
	.weak	SDL_MixAudio
	.weak	SDL_LockAudio
	.weak	SDL_UnlockAudio
	.weak	SDL_CloseAudio
	.weak	SDL_SetError
	.weak	SDL_GetError
	.weak	SDL_ClearError
	.weak	SDL_PeepEvents
	.weak	SDL_PumpEvents
	.weak	SDL_EventState
	.weak	IMG_Init
	.weak	IMG_Quit
	.weak	IMG_Load
	.weak	IMG_InvertAlpha
	.weak	SDL_EnableUNICODE
	.weak	SDL_EnableKeyRepeat
	.weak	SDL_GetKeyRepeat
	.weak	SDL_GetKeyState
	.weak	SDL_GetKeyName
	.weak	SDL_GetModState
	.weak	SDL_SetModState
	.weak	Mix_Init
	.weak	Mix_Quit
	.weak	Mix_OpenAudio
	.weak	Mix_AllocateChannels
	.weak	Mix_QuerySpec
	.weak	Mix_FreeChunk
	.weak	Mix_PlayChannelTimed
	.weak	Mix_Volume
	.weak	Mix_VolumeChunk
	.weak	Mix_VolumeMusic
	.weak	Mix_Pause
	.weak	Mix_Resume
	.weak	Mix_Paused
	.weak	Mix_Playing
	.weak	Mix_GetChunk
	.weak	Mix_CloseAudio
	.weak	SDL_GetMouseState
	.weak	SDL_GetRelativeMouseState
	.weak	SDL_WarpMouse
	.weak	SDL_CreateCursor
	.weak	SDL_SetCursor
	.weak	SDL_GetCursor
	.weak	SDL_FreeCursor
	.weak	SDL_ShowCursor
	.weak	SDL_GetTicks
	.weak	SDL_Delay
	.weak	SDL_WM_SetCaption
	.weak	SDL_SetVideoMode
	.weak	SDL_CreateRGBSurface
	.weak	SDL_Flip
	.weak	atof
	.weak	atoi
	.weak	atol
	.weak	atoll
	.weak	strtod
	.weak	strtof
	.weak	strtol
	.weak	strtoul
	.weak	strtoq
	.weak	strtouq
	.weak	initstate
	.weak	setstate
	.weak	rand
	.weak	srand
	.weak	rand_r
	.weak	drand48
	.weak	erand48
	.weak	lrand48
	.weak	nrand
	.weak	mrand48
	.weak	jrand48
	.weak	srand48
	.weak	seed48
	.weak	malloc
	.weak	calloc
	.weak	realloc
	.weak	free
	.weak	alloca
	.weak	abort
	.weak	exit
	.weak	putenv
	.weak	setenv
	.weak	unsetenv
	.weak	clearenv
	.weak	mktemp
	.weak	mkdtempt
	.weak	system
	.weak	realpath
	.weak	abs
	.weak	labs
	.weak	llabs
	.weak	mblen
	.weak	rename
	.weak	renameat
	.weak	tmpfile
	.weak	tmpnam
	.weak	tmpnam_r
	.weak	tempnam
	.weak	fclose
	.weak	fflush
	.weak	fflush_unlocked
	.weak	fcloseall
	.weak	fopen
	.weak	freopen
	.weak	setbuf
	.weak	setvbuf
	.weak	printf
	.weak	sprintf
	.weak	snprintf
	.weak	fscanf
	.weak	scanf
	.weak	fgetc
	.weak	getc
	.weak	getchar
	.weak	fputc
	.weak	putc
	.weak	putchar
	.weak	fgets
	.weak	gets
	.weak	fputs
	.weak	puts
	.weak	ungetc
	.weak	fread
	.weak	fwrite
	.weak	fseek
	.weak	ftell
	.weak	rewind
	.weak	feof
	.weak	ferror
	.weak	fileno

	.ident	"wlc 0.11 - Jan 2014 (GGJ)"
	.section	".note.GNU-stack","",@progbits
