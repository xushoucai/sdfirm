/*
 * ZETALOG's Personal COPYRIGHT
 *
 * Copyright (c) 2018
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
 * @(#)mmu.h: memory management unit definitions
 * $Id: mmu.h,v 1.279 2018-7-19 10:19:18 zhenglv Exp $
 */

#ifndef __MMU_ARM64_H_INCLUDE__
#define __MMU_ARM64_H_INCLUDE__

#include <target/barrier.h>
#include <target/sizes.h>

/*---------------------------------------------------------------------------
 * Hardware MMU definitions
 *---------------------------------------------------------------------------*/
/* Section address mask and size definitions */
#define SECTION_SHIFT		PMD_SHIFT
#define SECTION_SIZE		(_AC(1, ULL) << SECTION_SHIFT)
#define SECTION_MASK		(~(SECTION_SIZE-1))

/*
 * Contiguous page definitions.
 */
#ifdef CONFIG_MMU_64K_PAGE
#define CONT_PTE_SHIFT		5
#define CONT_PMD_SHIFT		5
#elif defined(CONFIG_MMU_16K_PAGE)
#define CONT_PTE_SHIFT		7
#define CONT_PMD_SHIFT		5
#else
#define CONT_PTE_SHIFT		4
#define CONT_PMD_SHIFT		4
#endif

/* Hardware page table definitions */

/* Level 1 descriptor (PUD) */
#define PUD_TYPE_TABLE		(_AT(pudval_t, 3) << 0)
#define PUD_TABLE_BIT		(_AT(pgdval_t, 1) << 1)
#define PUD_TYPE_MASK		(_AT(pgdval_t, 3) << 0)
#define PUD_TYPE_SECT		(_AT(pgdval_t, 1) << 0)

/* Level 2 descriptor (PMD) */
#define PMD_TYPE_MASK		(_AT(pmdval_t, 3) << 0)
#define PMD_TYPE_FAULT		(_AT(pmdval_t, 0) << 0)
#define PMD_TYPE_TABLE		(_AT(pmdval_t, 3) << 0)
#define PMD_TYPE_SECT		(_AT(pmdval_t, 1) << 0)
#define PMD_TABLE_BIT		(_AT(pmdval_t, 1) << 1)

/* Section */
#define PMD_SECT_VALID		(_AT(pmdval_t, 1) << 0)
#define PMD_SECT_PROT_NONE	(_AT(pmdval_t, 1) << 58)
#define PMD_SECT_USER		(_AT(pmdval_t, 1) << 6)		/* AP[1] */
#define PMD_SECT_RDONLY		(_AT(pmdval_t, 1) << 7)		/* AP[2] */
#define PMD_SECT_S		(_AT(pmdval_t, 3) << 8)
#define PMD_SECT_AF		(_AT(pmdval_t, 1) << 10)
#define PMD_SECT_NG		(_AT(pmdval_t, 1) << 11)
#define PMD_SECT_PXN		(_AT(pmdval_t, 1) << 53)
#define PMD_SECT_UXN		(_AT(pmdval_t, 1) << 54)

/* AttrIndx[2:0] encoding
 * (mapping attributes defined in the MAIR* registers)
 */
#define PMD_ATTRINDX(t)		(_AT(pmdval_t, (t)) << 2)
#define PMD_ATTRINDX_MASK	(_AT(pmdval_t, 7) << 2)

/* Level 3 descriptor (PTE) */
#define PTE_TYPE_MASK		(_AT(pteval_t, 3) << 0)
#define PTE_TYPE_FAULT		(_AT(pteval_t, 0) << 0)
#define PTE_TYPE_PAGE		(_AT(pteval_t, 3) << 0)
#define PTE_TABLE_BIT		(_AT(pteval_t, 1) << 1)
#define PTE_USER		(_AT(pteval_t, 1) << 6)		/* AP[1] */
#define PTE_RDONLY		(_AT(pteval_t, 1) << 7)		/* AP[2] */
#define PTE_SHARED		(_AT(pteval_t, 3) << 8)		/* SH[1:0], inner shareable */
#define PTE_AF			(_AT(pteval_t, 1) << 10)	/* Access Flag */
#define PTE_NG			(_AT(pteval_t, 1) << 11)	/* nG */
#define PTE_DBM			(_AT(pteval_t, 1) << 51)	/* Dirty Bit Management */
#define PTE_CONT		(_AT(pteval_t, 1) << 52)	/* Contiguous range */
#define PTE_PXN			(_AT(pteval_t, 1) << 53)	/* Privileged XN */
#define PTE_UXN			(_AT(pteval_t, 1) << 54)	/* User XN */

#define PTE_ADDR_LOW		(((_AT(pteval_t, 1) << (48 - PAGE_SHIFT)) - 1) << PAGE_SHIFT)
#ifdef CONFIG_CPU_64v8_2_LPA
#define PTE_ADDR_HIGH		(_AT(pteval_t, 0xf) << 12)
#define PTE_ADDR_MASK		(PTE_ADDR_LOW | PTE_ADDR_HIGH)
#else
#define PTE_ADDR_MASK		PTE_ADDR_LOW
#endif

/* AttrIndx[2:0] encoding
 * (mapping attributes defined in the MAIR* registers).
 */
#define PTE_ATTRINDX(t)		(_AT(pteval_t, (t)) << 2)
#define PTE_ATTRINDX_MASK	(_AT(pteval_t, 7) << 2)

/* 2nd stage PTE definitions */
#define PTE_S2_RDONLY		(_AT(pteval_t, 1) << 6)   /* HAP[2:1] */
#define PTE_S2_RDWR		(_AT(pteval_t, 3) << 6)   /* HAP[2:1] */
#define PMD_S2_RDWR		(_AT(pmdval_t, 3) << 6)   /* HAP[2:1] */

/* Memory Attribute override for Stage-2 (MemAttr[3:0]) */
#define PTE_S2_MEMATTR(t)	(_AT(pteval_t, (t)) << 2)
#define PTE_S2_MEMATTR_MASK	(_AT(pteval_t, 0xf) << 2)

/* EL2/HYP PTE/PMD definitions */
#define PMD_HYP			PMD_SECT_USER
#define PTE_HYP			PTE_USER

/*---------------------------------------------------------------------------
 * Software MMU definitions
 *---------------------------------------------------------------------------*/
/* Software defined PTE bits definition */
#define PTE_VALID		(_AT(pteval_t, 1) << 0)
#define PTE_FILE		(_AT(pteval_t, 1) << 2)	/* only when !pte_present() */
#define PTE_DIRTY		(_AT(pteval_t, 1) << 55)
#define PTE_SPECIAL		(_AT(pteval_t, 1) << 56)
#define PTE_WRITE		(_AT(pteval_t, 1) << 57)
#define PTE_PROT_NONE		(_AT(pteval_t, 1) << 58) /* only when !PTE_VALID */

#ifdef CONFIG_SMP
#define PROT_DEFAULT		(PTE_TYPE_PAGE | PTE_AF | PTE_SHARED)
#define PROT_SECT_DEFAULT	(PMD_TYPE_SECT | PMD_SECT_AF | PMD_SECT_S)
#else
#define PROT_DEFAULT		(PTE_TYPE_PAGE | PTE_AF)
#define PROT_SECT_DEFAULT	(PMD_TYPE_SECT | PMD_SECT_AF)
#endif

#define PROT_DEVICE_nGnRE	(PROT_DEFAULT | PTE_PXN | PTE_UXN | PTE_ATTRINDX(MT_DEVICE_nGnRE))
#define PROT_NORMAL_NC		(PROT_DEFAULT | PTE_PXN | PTE_UXN | PTE_ATTRINDX(MT_NORMAL_NC))
#define PROT_NORMAL		(PROT_DEFAULT | PTE_PXN | PTE_UXN | PTE_ATTRINDX(MT_NORMAL))

#define PROT_SECT_DEVICE_nGnRE	(PROT_SECT_DEFAULT | PMD_SECT_PXN | PMD_SECT_UXN | PMD_ATTRINDX(MT_DEVICE_nGnRE))
#define PROT_SECT_NORMAL	(PROT_SECT_DEFAULT | PMD_SECT_PXN | PMD_SECT_UXN | PMD_ATTRINDX(MT_NORMAL))
#define PROT_SECT_NORMAL_EXEC	(PROT_SECT_DEFAULT | PMD_SECT_UXN | PMD_ATTRINDX(MT_NORMAL))

#define _PAGE_DEFAULT		(PROT_DEFAULT | PTE_ATTRINDX(MT_NORMAL))

#define PAGE_KERNEL		__pgprot(_PAGE_DEFAULT | PTE_PXN | PTE_UXN | PTE_DIRTY | PTE_WRITE)
#define PAGE_KERNEL_RO		__pgprot((PROT_NORMAL & ~PTE_WRITE) | PTE_RDONLY)
#define PAGE_KERNEL_ROX		__pgprot((PROT_NORMAL & ~(PTE_WRITE | PTE_PXN)) | PTE_RDONLY)
#define PAGE_KERNEL_EXEC	__pgprot(_PAGE_DEFAULT | PTE_UXN | PTE_DIRTY | PTE_WRITE)
#define PAGE_KERNEL_IO		__pgprot(PROT_DEVICE_nGnRE)

#define PAGE_HYP		__pgprot(_PAGE_DEFAULT | PTE_HYP)
#define PAGE_HYP_DEVICE		__pgprot(PROT_DEVICE_nGnRE | PTE_HYP)

#define PAGE_S2			__pgprot(PROT_DEFAULT | PTE_S2_MEMATTR(MT_S2_NORMAL) | PTE_S2_RDONLY)
#define PAGE_S2_DEVICE		__pgprot(PROT_DEFAULT | PTE_S2_MEMATTR(MT_S2_DEVICE_nGnRE) | PTE_S2_RDONLY | PTE_UXN)

#define PAGE_NONE		__pgprot(((_PAGE_DEFAULT) & ~PTE_TYPE_MASK) | PTE_PROT_NONE | PTE_PXN | PTE_UXN)
#define PAGE_SHARED		__pgprot(_PAGE_DEFAULT | PTE_USER | PTE_NG | PTE_PXN | PTE_UXN | PTE_WRITE)
#define PAGE_SHARED_EXEC	__pgprot(_PAGE_DEFAULT | PTE_USER | PTE_NG | PTE_PXN | PTE_WRITE)
#define PAGE_COPY		__pgprot(_PAGE_DEFAULT | PTE_USER | PTE_NG | PTE_PXN | PTE_UXN)
#define PAGE_COPY_EXEC		__pgprot(_PAGE_DEFAULT | PTE_USER | PTE_NG | PTE_PXN)
#define PAGE_READONLY		__pgprot(_PAGE_DEFAULT | PTE_USER | PTE_NG | PTE_PXN | PTE_UXN)
#define PAGE_READONLY_EXEC	__pgprot(_PAGE_DEFAULT | PTE_USER | PTE_NG | PTE_PXN)

#define __P000  PAGE_NONE
#define __P001  PAGE_READONLY
#define __P010  PAGE_COPY
#define __P011  PAGE_COPY
#define __P100  PAGE_READONLY_EXEC
#define __P101  PAGE_READONLY_EXEC
#define __P110  PAGE_COPY_EXEC
#define __P111  PAGE_COPY_EXEC

#define __S000  PAGE_NONE
#define __S001  PAGE_READONLY
#define __S010  PAGE_SHARED
#define __S011  PAGE_SHARED
#define __S100  PAGE_READONLY_EXEC
#define __S101  PAGE_READONLY_EXEC
#define __S110  PAGE_SHARED_EXEC
#define __S111  PAGE_SHARED_EXEC

#define pgd_pfn(pgd)		((pgd_val(pgd) & PHYS_MASK) >> PAGE_SHIFT)
#define pfn_pgd(pfn, prot)	\
	__pgd(((phys_addr_t)(pfn) << PAGE_SHIFT) | pgprot_val(prot))
#if PGTABLE_LEVELS > 3
#define pgd_bad(pgd)		(!(pgd_val(pgd) & 2))
#define pgd_present(pgd)	pgd_val(pgd)
#endif /* PGTABLE_LEVELS > 3 */

#define pte_pfn(pte)		((pte_val(pte) & PHYS_MASK) >> PAGE_SHIFT)
#define pfn_pte(pfn, prot)	\
	__pte(((phys_addr_t)(pfn) << PAGE_SHIFT) | pgprot_val(prot))

/* The following only work if pte_present(). Undefined behaviour otherwise. */
#define pte_present(pte)	(!!(pte_val(pte) & (PTE_VALID | PTE_PROT_NONE)))
#define pte_dirty(pte)		(!!(pte_val(pte) & PTE_DIRTY))
#define pte_young(pte)		(!!(pte_val(pte) & PTE_AF))
#define pte_exec(pte)		(!(pte_val(pte) & PTE_UXN))

#ifndef __ASSEMBLY__
static inline pte_t clear_pte_bit(pte_t pte, pgprot_t prot)
{
	pte_val(pte) &= ~pgprot_val(prot);
	return pte;
}

static inline pte_t set_pte_bit(pte_t pte, pgprot_t prot)
{
	pte_val(pte) |= pgprot_val(prot);
	return pte;
}

static inline pte_t pte_wrprotect(pte_t pte)
{
	return clear_pte_bit(pte, __pgprot(PTE_WRITE));
}

static inline pte_t pte_mkwrite(pte_t pte)
{
	return set_pte_bit(pte, __pgprot(PTE_WRITE));
}

static inline pte_t pte_mkclean(pte_t pte)
{
	return clear_pte_bit(pte, __pgprot(PTE_DIRTY));
}

static inline pte_t pte_mkdirty(pte_t pte)
{
	return set_pte_bit(pte, __pgprot(PTE_DIRTY));
}

static inline pte_t pte_mkold(pte_t pte)
{
	return clear_pte_bit(pte, __pgprot(PTE_AF));
}

static inline pte_t pte_mkyoung(pte_t pte)
{
	return set_pte_bit(pte, __pgprot(PTE_AF));
}

#define pud_pte(pud)		__pte(pud_val(pud))
#define pud_pmd(pud)		__pmd(pud_val(pud))
#define pmd_pte(pmd)		__pte(pmd_val(pmd))
#define pte_pmd(pte)		__pmd(pte_val(pte))

#define pmd_young(pmd)		pte_young(pmd_pte(pmd))
#define pmd_wrprotect(pmd)	pte_pmd(pte_wrprotect(pmd_pte(pmd)))
#define pmd_mkold(pmd)		pte_pmd(pte_mkold(pmd_pte(pmd)))
#define pmd_mkwrite(pmd)	pte_pmd(pte_mkwrite(pmd_pte(pmd)))
#define pmd_mkdirty(pmd)	pte_pmd(pte_mkdirty(pmd_pte(pmd)))
#define pmd_mkyoung(pmd)	pte_pmd(pte_mkyoung(pmd_pte(pmd)))
#define pmd_mknotpresent(pmd)	(__pmd(pmd_val(pmd) & ~PMD_TYPE_MASK))

#define pmd_pfn(pmd)		((pmd_val(pmd) & PHYS_MASK) >> PAGE_SHIFT)
#define pfn_pmd(pfn, prot)	\
	__pmd(((phys_addr_t)(pfn) << PAGE_SHIFT) | pgprot_val(prot))

#define pmd_bad(pmd)		(!(pmd_val(pmd) & 2))
#define pmd_present(pmd)	pmd_val(pmd)

#define pud_pfn(pud)		((pud_val(pud) & PHYS_MASK) >> PAGE_SHIFT)
#define pfn_pud(pfn, prot)	\
	__pud(((phys_addr_t)(pfn) << PAGE_SHIFT) | pgprot_val(prot))
#if PGTABLE_LEVELS > 2
#define pud_bad(pud)		(!(pud_val(pud) & 2))
#define pud_present(pud)	(pud_val(pud))
#endif /* PGTABLE_LEVELS > 2 */

#define __pgprot_modify(prot,mask,bits)	\
	__pgprot((pgprot_val(prot) & ~(mask)) | (bits))

/* Mark the prot value as uncacheable and unbufferable. */
#define pgprot_noncached(prot)		\
	__pgprot_modify(prot, PTE_ATTRINDX_MASK, PTE_ATTRINDX(MT_DEVICE_nGnRnE) | PTE_PXN | PTE_UXN)
#define pgprot_writecombine(prot)	\
	__pgprot_modify(prot, PTE_ATTRINDX_MASK, PTE_ATTRINDX(MT_NORMAL_NC) | PTE_PXN | PTE_UXN)
#define pgprot_device(prot)		\
	__pgprot_modify(prot, PTE_ATTRINDX_MASK, PTE_ATTRINDX(MT_DEVICE_nGnRE) | PTE_PXN | PTE_UXN)
#define __HAVE_PHYS_MEM_ACCESS_PROT

#define pmd_table(pmd)		((pmd_val(pmd) & PMD_TYPE_MASK) == PMD_TYPE_TABLE)
#define pmd_sect(pmd)		((pmd_val(pmd) & PMD_TYPE_MASK) == PMD_TYPE_SECT)
#ifdef CONFIG_MMU_64K_PAGE
#define pud_sect(pud)		(0)
#else
#define pud_sect(pud)		((pud_val(pud) & PUD_TYPE_MASK) == PUD_TYPE_SECT)
#endif

static inline pte_t pte_modify(pte_t pte, pgprot_t newprot)
{
	const pteval_t mask = PTE_USER | PTE_PXN | PTE_UXN | PTE_RDONLY |
			      PTE_PROT_NONE | PTE_VALID | PTE_WRITE;
	pte_val(pte) = (pte_val(pte) & ~mask) | (pgprot_val(newprot) & mask);
	return pte;
}

static inline pgprot_t mk_pud_sect_prot(pgprot_t prot)
{
	return __pgprot((pgprot_val(prot) & ~PUD_TABLE_BIT) | PUD_TYPE_SECT);
}

static inline pgprot_t mk_pmd_sect_prot(pgprot_t prot)
{
	return __pgprot((pgprot_val(prot) & ~PMD_TABLE_BIT) | PMD_TYPE_SECT);
}

#define pmd_modify(pmd, newprot)	\
	pte_pmd(pte_modify(pmd_pte(pmd), newprot))
bool pgattr_change_is_safe(uint64_t old, uint64_t new);

/* To include device specific fixmaps */
#include <asm/mach/mmu.h>

void mmu_hw_create_mapping(phys_addr_t phys, caddr_t virt,
			   phys_addr_t size);
void mmu_hw_ctrl_init(void);
#endif /* !__ASSEMBLY__ */

#ifndef LINKER_SCRIPT
#ifdef __ASSEMBLY__
.extern mmu_id_map
.extern empty_zero_page
#else
extern pgd_t mmu_id_map[IDMAP_DIR_SIZE / sizeof (pgd_t)];
extern unsigned long empty_zero_page[PAGE_SIZE / sizeof (unsigned long)];
#endif
#endif

#if defined(__ASSEMBLY__) && !defined(LINKER_SCRIPT)
/*
 * Macro to create a table entry to the next page.
 *	tbl:	page table address
 *	virt:	virtual address
 *	shift:	#imm page table shift
 *	ptrs:	#imm pointers per table page
 * Preserves:	virt
 * Corrupts:	tmp1, tmp2
 * Returns:	tbl -> next level table page address
 */
	.macro	create_table_entry, tbl, virt, shift, ptrs, tmp1, tmp2
	lsr	\tmp1, \virt, #\shift
	and	\tmp1, \tmp1, #\ptrs - 1	// table index
	add	\tmp2, \tbl, #PAGE_SIZE
	orr	\tmp2, \tmp2, #PMD_TYPE_TABLE	// address of next table and entry type
	str	\tmp2, [\tbl, \tmp1, lsl #3]
	add	\tbl, \tbl, #PAGE_SIZE		// next level table page
	.endm

/*
 * Macro to populate the PGD (and possibily PUD) for the corresponding
 * block entry in the next level (tbl) for the given virtual address.
 * Preserves:	tbl, next, virt
 * Corrupts:	tmp1, tmp2
 */
	.macro	create_pgd_entry, tbl, virt, tmp1, tmp2
	create_table_entry \tbl, \virt, PGDIR_SHIFT, PTRS_PER_PGD, \tmp1, \tmp2
#if BPGT_PGTABLE_LEVELS > 3
	create_table_entry \tbl, \virt, PUD_SHIFT, PTRS_PER_PUD, \tmp1, \tmp2
#endif
#if BPGT_PGTABLE_LEVELS > 2
	create_table_entry \tbl, \virt, BPGT_TABLE_SHIFT, PTRS_PER_PTE, \tmp1, \tmp2
#endif
	.endm

	.macro	locate_pgd_entry, tbl
	add	\tbl, \tbl, #PAGE_SIZE		// next level table page
#if BPGT_PGTABLE_LEVELS > 3
	add	\tbl, \tbl, #PAGE_SIZE		// next level table page
#endif
#if BPGT_PGTABLE_LEVELS > 2
	add	\tbl, \tbl, #PAGE_SIZE		// next level table page
#endif
	.endm

/*
 * Macro to populate block entries in the page table for the start..end
 * virtual range (inclusive).
 * Preserves:	tbl, flags
 * Corrupts:	phys, start, end, pstate
 */
	.macro	create_block_map, tbl, flags, phys, start, end
	lsr	\phys, \phys, #BPGT_BLOCK_SHIFT
	lsr	\start, \start, #BPGT_BLOCK_SHIFT
	and	\start, \start, #PTRS_PER_PTE - 1		// table index
	orr	\phys, \flags, \phys, lsl #BPGT_BLOCK_SHIFT	// table entry
	lsr	\end, \end, #BPGT_BLOCK_SHIFT
	and	\end, \end, #PTRS_PER_PTE - 1			// table end index
9999:	str	\phys, [\tbl, \start, lsl #3]			// store the entry
	add	\start, \start, #1				// next entry
	add	\phys, \phys, #BPGT_BLOCK_SIZE			// next block
	cmp	\start, \end
	b.ls	9999b
	.endm
#endif /* __ASSEMBLY__ && !LINKER_SCRIPT */

#endif /* __MMU_ARM64_H_INCLUDE__ */
