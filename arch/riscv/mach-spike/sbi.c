#if 0
#include <libfdt.h>
#include <fdt.h>
#endif
#include <target/arch.h>
#include <target/uart.h>
#include <target/irq.h>
#include <target/delay.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_platform.h>

#define SPIKE_HART_STACK_SIZE			8192

static void spike_modify_dt(void *fdt)
{
}

static int spike_final_init(bool cold_boot)
{
	void *fdt;

	if (!cold_boot)
		return 0;

	fdt = sbi_scratch_thishart_arg1_ptr();
	spike_modify_dt(fdt);
	return 0;
}

static u32 spike_pmp_region_count(u32 hartid)
{
	return 1;
}

static int spike_pmp_region_info(u32 hartid, u32 index, ulong *prot,
				 ulong *addr, ulong *log2size)
{
	int ret = 0;

	switch (index) {
	case 0:
		*prot	  = PMP_R | PMP_W | PMP_X;
		*addr	  = 0;
		*log2size = __riscv_xlen;
		break;
	default:
		ret = -1;
		break;
	};
	return ret;
}

void spike_console_putc(char ch)
{
	uart_hw_con_write(ch);
}

int spike_console_getc(void)
{
	while (!uart_hw_con_poll());
	return (int)uart_hw_con_read();
}

static int spike_console_init(void)
{
	uart_hw_con_init();
	return 0;
}

static int spike_irqchip_init(bool cold_boot)
{
	return 0;
}

void spike_ipi_send(u32 target_hart)
{
	clint_set_ipi(target_hart);
}

void spike_ipi_sync(u32 target_hart)
{
	clint_sync_ipi(target_hart);
}

void spike_ipi_clear(u32 target_hart)
{
	clint_clear_ipi(target_hart);
}

static int spike_ipi_init(bool cold_boot)
{
	cpu_t cpu = sbi_current_hartid();

	if (!cold_boot)
		spike_ipi_clear(cpu);
	return 0;
}

u64 spike_timer_value(void)
{
	return clint_read_mtime();
}

void spike_timer_event_stop(void)
{
	cpu_t cpu = sbi_current_hartid();

	clint_unset_mtimecmp(cpu);
}

void spike_timer_event_start(u64 next_event)
{
	cpu_t cpu = sbi_current_hartid();

	clint_set_mtimecmp(cpu, next_event);
}

static int spike_timer_init(bool cold_boot)
{
	if (!cold_boot)
		spike_timer_event_stop();
	return 0;
}

static int spike_system_down(u32 type)
{
	/* For now nothing to do. */
	return 0;
}

const struct sbi_platform_operations platform_ops = {
	.pmp_region_count	= spike_pmp_region_count,
	.pmp_region_info	= spike_pmp_region_info,
	.final_init		= spike_final_init,
	.console_putc		= spike_console_putc,
	.console_getc		= spike_console_getc,
	.console_init		= spike_console_init,
	.irqchip_init		= spike_irqchip_init,
	.ipi_send		= spike_ipi_send,
	.ipi_sync		= spike_ipi_sync,
	.ipi_clear		= spike_ipi_clear,
	.ipi_init		= spike_ipi_init,
	.timer_value		= spike_timer_value,
	.timer_event_stop	= spike_timer_event_stop,
	.timer_event_start	= spike_timer_event_start,
	.timer_init		= spike_timer_init,
	.system_reboot		= spike_system_down,
	.system_shutdown	= spike_system_down
};

const struct sbi_platform platform = {
	.opensbi_version	= OPENSBI_VERSION,
	.platform_version	= SBI_PLATFORM_VERSION(0x0, 0x01),
	.name			= "RISC-V ISA simulator (spike)",
	.features		= SBI_PLATFORM_DEFAULT_FEATURES,
	.hart_count		= NR_CPUS,
	.hart_stack_size	= SPIKE_HART_STACK_SIZE,
	.disabled_hart_mask	= 0,
	.platform_ops_addr	= (unsigned long)&platform_ops
};
