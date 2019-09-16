#ifndef __TASK_RISCV_H_INCLUDE__
#define __TASK_RISCV_H_INCLUDE__

#include <target/compiler.h>
#include <target/page.h>

#define MACHINE_STACK_SIZE		PAGE_SIZE
#define MENTRY_HLS_OFFSET		\
	(INTEGER_CONTEXT_SIZE + SOFT_FLOAT_CONTEXT_SIZE)
#define MENTRY_FRAME_SIZE		(MENTRY_HLS_OFFSET + HLS_SIZE)
#define MENTRY_IPI_OFFSET		(MENTRY_HLS_OFFSET)
#define MENTRY_IPI_PENDING_OFFSET	(MENTRY_HLS_OFFSET + SZREG)

#ifdef __riscv_flen
# define SOFT_FLOAT_CONTEXT_SIZE	0
#else
# define SOFT_FLOAT_CONTEXT_SIZE	(8 * 32)
#endif
#define HLS_SIZE			64
#define INTEGER_CONTEXT_SIZE		(32 * SZREG)

#endif /* __TASK_RISCV_H_INCLUDE__ */
