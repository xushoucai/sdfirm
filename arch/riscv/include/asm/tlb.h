/*
 * ZETALOG's Personal COPYRIGHT
 *
 * Copyright (c) 2019
 *    ZETALOG - "Lv ZHENG".  All rights reserved.
 *    Author: Lv "Zetalog" Zheng
 *    Internet: zhenglv@hotmail.com
 *
 * This COPYRIGHT used to protect Personal Intelligence Rights.
 * Redistribution and use in source and binary forms with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the Lv "Zetalog" ZHENG.
 * 3. Neither the name of this software nor the names of its developers may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 4. Permission of redistribution and/or reuse of souce code partially only
 *    granted to the developer(s) in the companies ZETALOG worked.
 * 5. Any modification of this software should be published to ZETALOG unless
 *    the above copyright notice is no longer declaimed.
 *
 * THIS SOFTWARE IS PROVIDED BY THE ZETALOG AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE ZETALOG OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @(#)tlb.h: RISCV TLB flush interfaces
 * $Id: tlb.h,v 1.1 2019-08-14 14:20:00 zhenglv Exp $
 */

#ifndef __TLB_RISCV_H_INCLUDE__
#define __TLB_RISCV_H_INCLUDE__

#include <asm/sbi.h>

#ifndef __ASSEMBLY__
#define local_flush_tlb_all()		\
	asm volatile ("sfence.vma" : : : "memory")
#define local_flush_tlb_page(addr)	\
	asm volatile ("sfence.vma %0" : : "r" (addr) : "memory")

#ifndef CONFIG_SMP
#define flush_tlb_all()			local_flush_tlb_all()
#define flush_tlb_page(asid, addr)	local_flush_tlb_page(addr)
#define flush_tlb_range_user(asid, start, end)		\
	local_flush_tlb_all()
#else
#define flush_tlb_all()					\
	sbi_remote_sfence_vma(CPU_ALL, 0, -1)
#define flush_tlb_page(asid, addr)			\
	sbi_remote_sfence_vma(asid2cpu(asid), (addr), 0)
#define flush_tlb_range_user(asid, start, end)		\
	sbi_remote_sfence_vma(asid2cpu(asid), (start), (end) - (start))
#endif /* CONFIG_SMP */

#define flush_tlb_range_kern(start, end)	 flush_tlb_all()
#endif /* !__ASSEMBLY__ */

#endif /* __TLB_RISCV_H_INCLUDE__ */
