/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/sbi_console.h>
#include <sbi/sbi_ecall.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_ipi.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_system.h>
#include <sbi/sbi_timer.h>
#include <sbi/sbi_version.h>
#include <target/smp.h>
#include <target/atomic.h>

#define BANNER                                              \
	"   ____                    _____ ____ _____\n"     \
	"  / __ \\                  / ____|  _ \\_   _|\n"  \
	" | |  | |_ __   ___ _ __ | (___ | |_) || |\n"      \
	" | |  | | '_ \\ / _ \\ '_ \\ \\___ \\|  _ < | |\n" \
	" | |__| | |_) |  __/ | | |____) | |_) || |_\n"     \
	"  \\____/| .__/ \\___|_| |_|_____/|____/_____|\n"  \
	"        | |\n"                                     \
	"        |_|\n\n"

#if 0
struct sbi_scratch *sbi_scratches[NR_CPUS];
#else
struct sbi_scratch *sbi_scratches[5]; /* for fu540 */
#endif

void sbi_late_init(void)
{
	char str[64];
	cpu_t hartid = sbi_current_hartid();
	struct sbi_scratch *scratch = sbi_scratches[hartid];
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);

#if 0
	if (!(scratch->options & SBI_SCRATCH_NO_BOOT_PRINTS))
		return;
#endif

	misa_string(str, sizeof(str));
	sbi_printf("\nOpenSBI v%d.%d (%s %s)\n", OPENSBI_VERSION_MAJOR,
		   OPENSBI_VERSION_MINOR, __DATE__, __TIME__);

	sbi_printf(BANNER);

	/* Platform details */
	sbi_printf("Platform Name          : %s\n", sbi_platform_name(plat));
	sbi_printf("Platform HART Features : RV%d%s\n", misa_xlen(), str);
	sbi_printf("Platform Max HARTs     : %d\n",
		   sbi_platform_hart_count(plat));
	sbi_printf("Current Hart           : %u\n", hartid);
	/* Firmware details */
	sbi_printf("Firmware Base          : 0x%lx\n", scratch->fw_start);
	sbi_printf("Firmware Size          : %d KB\n",
		   (u32)(scratch->fw_size / 1024));
	/* Generic details */
	sbi_printf("Runtime SBI Version    : %d.%d\n",
		   sbi_ecall_version_major(), sbi_ecall_version_minor());
	sbi_printf("\n");

	pmp_dump(0, NULL);
}

static void __noreturn init_coldboot(void)
{
	int rc;
	cpu_t hartid = sbi_current_hartid();
	struct sbi_scratch *scratch = sbi_scratches[hartid];
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);

	rc = sbi_system_early_init(scratch, true);
	if (rc)
		sbi_hart_hang();

	rc = sbi_hart_init(scratch, hartid, true);
	if (rc)
		sbi_hart_hang();

	rc = sbi_console_init(scratch);
	if (rc)
		sbi_hart_hang();

	rc = sbi_platform_irqchip_init(plat, true);
	if (rc)
		sbi_hart_hang();

	rc = sbi_ipi_init(scratch, true);
	if (rc)
		sbi_hart_hang();

	rc = sbi_timer_init(scratch, true);
	if (rc)
		sbi_hart_hang();

	rc = sbi_system_final_init(scratch, true);
	if (rc)
		sbi_hart_hang();

	if (!sbi_platform_has_hart_hotplug(plat))
		sbi_hart_wake_coldboot_harts(scratch, hartid);
	sbi_hart_mark_available(hartid);
	sbi_late_init();
	sbi_hart_switch_mode(hartid, scratch->next_arg1, scratch->next_addr,
			     scratch->next_mode);
}

static void __noreturn init_warmboot(void)
{
	int rc;
	cpu_t hartid = sbi_current_hartid();
	struct sbi_scratch *scratch = sbi_scratches[hartid];
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);

	if (!sbi_platform_has_hart_hotplug(plat))
		sbi_hart_wait_for_coldboot(scratch, hartid);

	if (sbi_platform_hart_disabled(plat, hartid))
		sbi_hart_hang();

	rc = sbi_system_early_init(scratch, false);
	if (rc)
		sbi_hart_hang();

	rc = sbi_hart_init(scratch, hartid, false);
	if (rc)
		sbi_hart_hang();

	rc = sbi_platform_irqchip_init(plat, false);
	if (rc)
		sbi_hart_hang();

	rc = sbi_ipi_init(scratch, false);
	if (rc)
		sbi_hart_hang();

	rc = sbi_timer_init(scratch, false);
	if (rc)
		sbi_hart_hang();

	rc = sbi_system_final_init(scratch, false);
	if (rc)
		sbi_hart_hang();

	sbi_hart_mark_available(hartid);

	if (sbi_platform_has_hart_hotplug(plat))
		/* TODO: To be implemented in-future. */
		sbi_hart_hang();
	else
		sbi_hart_switch_mode(hartid, scratch->next_arg1,
				     scratch->next_addr, scratch->next_mode);
}

static atomic_t coldboot_lottery = ATOMIC_INIT(0);

/**
 * Initialize OpenSBI library for current HART and jump to next
 * booting stage.
 *
 * The function expects following:
 * 1. The 'mscratch' CSR is pointing to sbi_scratch of current HART
 * 2. Stack pointer (SP) is setup for current HART
 * 3. Interrupts are disabled in MSTATUS CSR
 * 4. All interrupts are disabled in MIE CSR
 *
 * @param scratch pointer to sbi_scratch of current HART
 */
void __noreturn sbi_init(void)
{
	cpu_t hartid = sbi_current_hartid();
	struct sbi_scratch *scratch =
		(struct sbi_scratch *)csr_read(CSR_MSCRATCH);
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);
	bool coldboot = false;

	if (sbi_platform_hart_disabled(plat, hartid))
		sbi_hart_hang();

	sbi_scratches[hartid] = scratch;

	if (atomic_add_return(1, &coldboot_lottery) == 1)
		coldboot = true;

	if (coldboot)
		init_coldboot();
	else
		init_warmboot();
}
