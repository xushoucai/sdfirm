#include <target/init.h>
#include <target/arch.h>
#include <target/paging.h>

	.macro	init_sp
	la	sp, PERCPU_STACKS_END
	.endm

	.macro	init_gp
	.option push
	.option norelax
	la	gp, __global_pointer$
	.option pop
	.endm

	.macro disable_mirq
#ifdef CONFIG_RISCV_ENTR_M
	csrci	mstatus, SR_MIE
#endif
	.endm

	.macro disable_sirq
	csrci	sstatus, SR_MIE
	.endm

	.macro disable_fpu
#ifdef CONFIG_CPU_F
	li	t0, SR_FS
	csrc	sstatus, t0
#endif
	.endm

	.macro	zerow regs
	REG_S	zero, 0(\regs)
	.endm

	.macro	copyw regl, regs, regv
	REG_L	\regv, 0(\regl)
	REG_S	\regv, 0(\regs)
	.endm

	.macro	zero_sect, sva, eva, lab_exit, lab_loop
	la	a0, \sva
	la	a1, \eva
	j	\lab_exit
\lab_loop:
	zerow	a0
	addi	a0, a0, WORD_SIZE
\lab_exit:
	bltu	a0, a1, \lab_loop
	.endm

	.macro	copy_sect, sla, sva, eva, lab_exit, lab_loop
	la	a2, \sla
	la	a0, \sva
	la	a1, \eva
	j	\lab_exit
\lab_loop:
	copyw	a2, a0, a3
	addi	a2, a2, WORD_SIZE
	addi	a0, a0, WORD_SIZE
\lab_exit:
	bltu	a0, a1, \lab_loop
	.endm

	__HEAD

ENTRY(__start)
	/* Disable global interrupt. */
	disable_mirq

#ifdef CONFIG_LOAD_TEXT
	/* Handle both .text and .rodata sections */
	copy_sect __text_loc, _stext, _etext, copy_text_exit, copy_text_loop
#endif
#ifdef CONFIG_LOAD_DATA
	/* Handle .data section */
	copy_sect __data_loc, __sdata, __edata, copy_data_exit, copy_data_loop
#endif
	/* Handle .bss section */
	zero_sect __bss_start, __bss_stop, init_bss_exit, init_bss_loop

	fence	rw, rw
	/* At this point we are running from link address */

	/* Reset all registers for boot HART */
	li	ra, 0
	call	reset_gprs

#ifdef CONFIG_SBI
	jal	ra, __sbi_entry
#else
	la	t0, __vectors
	csrw	mtvec, t0
#ifdef CONFIG_RISCV_N
	/* csrw	utvec, t0 */
	csrw	CSR_UTVEC, t0
#endif

	jal	ra, system_init
#endif
ENDPROC(__start)

ENTRY(__sbi_exit)
	disable_sirq

#ifdef CONFIG_MMU
	call	mmu_hw_boot_init
#endif

	jal	ra, system_init
ENDPROC(__sbi_exit)

ENTRY(reset_gprs)
	/* flush the instruction cache */
	fence.i
	/* Reset all registers except ra, a0, a1 and a2 */
	init_sp
	init_gp
	li	tp, 0
	li	t0, 0
	li	t1, 0
	li	t2, 0
	li	s0, 0
	li	s1, 0
	li	a3, 0
	li	a4, 0
	li	a5, 0
	li	a6, 0
	li	a7, 0
	li	s2, 0
	li	s3, 0
	li	s4, 0
	li	s5, 0
	li	s6, 0
	li	s7, 0
	li	s8, 0
	li	s9, 0
	li	s10, 0
	li	s11, 0
	li	t3, 0
	li	t4, 0
	li	t5, 0
	li	t6, 0
	csrw	CSR_MSCRATCH, 0
	ret
ENDPROC(reset_gprs)
