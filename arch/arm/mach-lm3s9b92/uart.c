#include <target/uart.h>
#include <target/irq.h>

uint32_t __uart_hw_config_param(uint8_t params)
{
	uint32_t cfg;

	cfg = (uart_bits(params)-5) << WLEN;
	switch (uart_parity(params)) {
	case UART_PARITY_EVEN:
		cfg |= _BV(EPS);
	case UART_PARITY_ODD:
		cfg |= _BV(PEN);
		break;
	}
	if (uart_stopb(params))
		cfg |= _BV(STP2);

	return cfg;
}

void __uart_hw_ctrl_disable(uint8_t n)
{
	while (__raw_readl(UARTFR(n)) & _BV(BUSY));
	/* disable the UART */
	__raw_clearl(_BV(UARTEN) | _BV(TXE) | _BV(RXE), UARTCTL(n));
}

void __uart_hw_ctrl_enable(uint8_t n, uint8_t params)
{
	/* disable the FIFO and BRK */
	__raw_writel(__uart_hw_config_param(params),
		     UARTLCRH(n));
	/* enable RX, TX, and the UART */
	__raw_writel(_BV(UARTEN) | _BV(TXE) | _BV(RXE), UARTCTL(n));
	/* clear the flags register */
	/* __raw_writel(0, UART0FR); */
}

static void __uart_hw_config_baudrate(uint8_t n, uint32_t baudrate)
{
	unsigned long cfg;

	/* is the required baud rate greater than the maximum rate */
	/* supported without the use of high speed mode? */
	if (baudrate > mul16u((uint16_t)div32u(CLK_SYS, 16), 1000)) {
		/* enable high speed mode */
		__raw_setl_atomic(HSE, UARTCTL(n));
		baudrate = div32u(baudrate, 2);
	} else {
		/* disable high speed mode */
		__raw_clearl_atomic(HSE, UARTCTL(n));
	}
	cfg = div32u(div32u((uint32_t)(mul32u(CLK_SYS, 20)),
			    div32u(baudrate, 400)) + 1, 2);
	/* set the baud rate */
	__raw_writel(cfg >> __UART_HW_FBRD_OFFSET, UARTIBRD(n));
	__raw_writel(cfg & __UART_HW_FBRD_MASK, UARTFBRD(n));
}

void __uart_hw_ctrl_config(uint8_t n,
			   uint8_t params,
			   uint32_t baudrate)
{
	__uart_hw_config_baudrate(n, baudrate);
	/* UARTLCRH write must follows UART(I|F)BRD writes */
	__raw_writel_mask(__uart_hw_config_param(params), 0xEE,
			  UARTLCRH(n));
}

#ifdef CONFIG_UART_SYNC
static inline void __uart_hw_config_pins(void)
{
	pm_hw_resume_device(DEV_GPIOA, DEV_MODE_ON);
	/* configure UART0 RX pin */
	gpio_config_mux(GPIOA, 0, GPIOA0_MUX_U0RX);
	gpio_config_pad(GPIOA, 0, GPIO_PAD_PP, 2);
	/* configure UART0 TX pin */
	gpio_config_mux(GPIOA, 1, GPIOA1_MUX_U0TX);
	gpio_config_pad(GPIOA, 1, GPIO_PAD_PP, 2);
}

void uart_hw_sync_write(uint8_t byte)
{
	if (pm_hw_device_mode(DEV_UART0) != DEV_MODE_OFF)
		__uart_hw_sync_write(0, byte);
}

uint8_t uart_hw_sync_read(void)
{
	if (pm_hw_device_mode(DEV_UART0) != DEV_MODE_OFF)
		return __uart_hw_sync_read(0);
	return 0;
}

void uart_hw_sync_start(void)
{
	__uart_hw_ctrl_enable(0, UART_SYNC_PARAMS);
}

void uart_hw_sync_stop(void)
{
	__uart_hw_ctrl_disable(0);
}

void uart_hw_sync_config(uint8_t params,
			 uint32_t baudrate)
{
	__uart_hw_ctrl_config(0, params, baudrate);
}

void uart_hw_sync_init(void)
{
	pm_hw_resume_device(DEV_UART0, DEV_MODE_ON);
	__uart_hw_config_pins();
}
#endif

#ifdef CONFIG_UART_ASYNC
#define __UART_HW_RX_IRQS	(_BV(RTI) | _BV(RXI))
#define __UART_HW_TX_IRQS	(_BV(TXI))

struct uart_hw_gpio uart_hw_gpios[NR_UART_PORTS] = {
	{__UART0_HW_DEV_GPIO, __UART0_HW_DEV_UART,
	 __UART0_HW_RX_PORT, __UART0_HW_RX_PIN, __UART0_HW_RX_MUX,
	 __UART0_HW_TX_PORT, __UART0_HW_TX_PIN, __UART0_HW_TX_MUX},
#if NR_UART_PORTS > 1
	{__UART1_HW_DEV_GPIO, __UART1_HW_DEV_UART,
	 __UART1_HW_RX_PORT, __UART1_HW_RX_PIN, __UART1_HW_RX_MUX,
	 __UART1_HW_TX_PORT, __UART1_HW_TX_PIN, __UART1_HW_TX_MUX},
#endif
#if NR_UART_PORTS > 2
	{__UART2_HW_DEV_GPIO, __UART2_HW_DEV_UART,
	 __UART2_HW_RX_PORT, __UART2_HW_RX_PIN, __UART2_HW_RX_MUX,
	 __UART2_HW_TX_PORT, __UART2_HW_TX_PIN, __UART2_HW_TX_MUX},
#endif
};

static uart_pid_t __uart_hw_pids[NR_UART_PORTS];
static uart_pid_t __uart_hw_pid;

static void uart_hw_async_dummy(void)
{
}

static boolean uart_hw_rx_poll(void)
{
	return __uart_hw_read_empty(__uart_hw_pid);
}

static boolean uart_hw_tx_poll(void)
{
	return !__uart_hw_write_full(__uart_hw_pid);
}

static void uart_hw_rx_getch(uint8_t *byte)
{
	*byte = __uart_hw_async_read(__uart_hw_pid);
}

static void uart_hw_tx_putch(uint8_t *byte)
{
	__uart_hw_async_write(__uart_hw_pid, *byte);
}

static void uart_hw_rx_start(void)
{
	if (bulk_request_syncing()) {
		while (!__uart_hw_irq_raised(__uart_hw_pid,
					     __UART_HW_RX_IRQS));
	}
	uart_read_submit(__uart_hw_pid, 1);
}

static void uart_hw_rx_stop(void)
{
	if (__uart_hw_irq_raised(__uart_hw_pid,
				 __UART_HW_RX_IRQS)) {
		__uart_hw_irq_unraise(__uart_hw_pid,
				      __UART_HW_RX_IRQS);
	}
}

static void uart_hw_tx_start(void)
{
	if (bulk_request_syncing()) {
		while (!__uart_hw_irq_raised(__uart_hw_pid,
					     __UART_HW_TX_IRQS));
	}
}

static void uart_hw_tx_stop(void)
{
	if (bulk_request_syncing()) {
		while (!__uart_hw_irq_raised(__uart_hw_pid,
					     __UART_HW_TX_IRQS));
		__uart_hw_irq_unraise(__uart_hw_pid,
				      __UART_HW_TX_IRQS);
	}
}

static void uart_hw_handle_irq(void)
{
}

#ifdef SYS_REALTIME
#define uart_hw_tx_open		uart_hw_async_dummy
#define uart_hw_tx_close	uart_hw_async_dummy
#define uart_hw_rx_open		uart_hw_async_dummy
#define uart_hw_rx_close	uart_hw_async_dummy
#define uart_hw_async_start	uart_hw_async_dummy
#define uart_hw_async_stop	uart_hw_async_dummy

void uart_hw_irq_poll(void)
{
	uart_hw_handle_irq();
}
#else
static void uart_hw_tx_open(void)
{
	__uart_hw_irq_enable(__uart_hw_pid, __UART_HW_TX_IRQS);
}

static void uart_hw_tx_close(void)
{
	__uart_hw_irq_disable(__uart_hw_pid, __UART_HW_TX_IRQS);
}

static void uart_hw_rx_open(void)
{
	__uart_hw_irq_enable(__uart_hw_pid, __UART_HW_RX_IRQS);
}

static void uart_hw_rx_close(void)
{
	__uart_hw_irq_disable(__uart_hw_pid, __UART_HW_RX_IRQS);
}

static void uart_hw_rx_halt(void)
{
	uart_read_sync(__uart_hw_pid);
}

static void __uart_hw_handle_irq(void)
{
	uart_hw_handle_irq();
}

uint8_t __uart_hw_port_irq(void)
{
	if (__uart_hw_pid == 2)
		return IRQ_UART2;
	else
		return IRQ_UART0+__uart_hw_pid;
}

void uart_hw_async_start(void)
{
	uint8_t irq;

	irq = __uart_hw_port_irq();
	vic_hw_register_irq(irq, __uart_hw_handle_irq);
	vic_hw_irq_enable(irq);
}

void uart_hw_async_stop(void)
{
	uint8_t irq;

	irq = __uart_hw_port_irq();
	vic_hw_irq_disable(irq);
}
#endif

void uart_hw_async_config(uint8_t params,
			uint32_t baudrate)
{
	__uart_hw_ctrl_config(__uart_hw_pid, params, baudrate);
}

bulk_channel_t __uart_hw_tx = {
	O_WRONLY,
	1,
	uart_hw_tx_open,
	uart_hw_tx_close,
	uart_hw_tx_start,
	uart_hw_tx_stop,
	uart_hw_async_dummy,
	uart_hw_async_dummy,
	uart_hw_tx_poll,
	uart_hw_tx_putch,
	NULL,
};

bulk_channel_t __uart_hw_rx = {
	O_RDONLY,
	1,
	uart_hw_rx_open,
	uart_hw_rx_close,
	uart_hw_rx_start,
	uart_hw_rx_stop,
	uart_hw_rx_halt,
	uart_hw_async_dummy,
	uart_hw_rx_poll,
	uart_hw_rx_getch,
	NULL,
};

static uart_port_t __uart_hw_port = {
	uart_hw_async_start,
	uart_hw_async_stop,
	uart_hw_async_config,
	(bulk_channel_t *)(&__uart_hw_tx),
	(bulk_channel_t *)(&__uart_hw_rx),
};

void uart_hw_async_select(uart_pid_t pid)
{
	uint8_t n;

	for (n = 0; n < NR_UART_PORTS; n++) {
		if (__uart_hw_pids[n] == pid) {
			__uart_hw_pid = pid;
			break;
		}
	}
}

void uart_hw_async_init(void)
{
	uint8_t n;

	for (n = 0; n < NR_UART_PORTS; n++) {
		/* enable GPIO for PIN configurations */
		pm_hw_resume_device(uart_hw_gpios[n].gpio, DEV_MODE_ON);
		/* configure UART0 RX pin */
		gpio_config_mux(uart_hw_gpios[n].rx_port,
				uart_hw_gpios[n].rx_pin,
				uart_hw_gpios[n].rx_mux);
		gpio_config_pad(uart_hw_gpios[n].rx_port,
				uart_hw_gpios[n].rx_pin,
				GPIO_PAD_PP, 2);
		/* configure UART0 TX pin */
		gpio_config_mux(uart_hw_gpios[n].tx_port,
				uart_hw_gpios[n].tx_pin,
				uart_hw_gpios[n].tx_mux);
		gpio_config_pad(uart_hw_gpios[n].tx_port,
				uart_hw_gpios[n].tx_pin,
				GPIO_PAD_PP, 2);
		/* enable UART port */
		pm_hw_resume_device(uart_hw_gpios[n].uart, DEV_MODE_ON);
		/* register uart port */
		__uart_hw_pids[n] = uart_register_port(&__uart_hw_port);
	}
}
#endif
