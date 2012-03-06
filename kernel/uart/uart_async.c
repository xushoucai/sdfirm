#include <target/uart.h>
#include <target/generic.h>

struct uart_state {
	bulk_cid_t bulk_in;
	bulk_cid_t bulk_out;
};

uart_port_t *uart_ports[NR_UART_PORTS];
uart_pid_t uart_nr_ports;
uart_pid_t uart_pid;

struct uart_state uart_states[NR_UART_PORTS];
DECLARE_BITMAP(uart_port_regs, NR_UART_PORTS);

void uart_port_restore(uart_pid_t pid)
{
	uart_pid = pid;
}

uart_pid_t uart_port_save(uart_pid_t pid)
{
	uart_pid_t spid = uart_pid;
	uart_port_restore(pid);
	return spid;
}

boolean uart_started(uart_pid_t pid)
{
	return test_bit(pid, uart_port_regs);
}

void uart_write_wakeup(void)
{
	/* bulk_write_aval(uart_ctrl[uart_pid].bulk); */
}

void uart_stop(void)
{
	uart_port_t *port = uart_ports[uart_pid];

	BUG_ON(!port || !port->stop_tx);
	port->stop_tx();
}

void uart_start(void)
{
	uart_port_t *port = uart_ports[uart_pid];

	BUG_ON(!port || !port->start_tx);
#if 0
	if (!uart_circ_empty(&state->xmit) && state->xmit.buf)
		port->start_tx(port);
#endif
}

uart_pid_t uart_startup(uint8_t *inbuf, int inlen,
			uint8_t *outbuf, int outlen)
{
	uart_port_t *port;
	uart_pid_t pid;

	for (pid = 0; pid < NR_UART_PORTS; pid++) {
		if (!uart_started(pid)) {
			uart_port_select(pid);
			port = uart_ports[uart_pid];
			uart_states[uart_pid].bulk_out =
				bulk_set_buffer(outbuf, outlen);
			uart_states[uart_pid].bulk_in =
				bulk_set_buffer(inbuf, inlen);
			BUG_ON(!port || !port->startup);
			port->startup();
			set_bit(pid, uart_port_regs);
			break;
		}
	}

	return pid;
}

void uart_cleanup(uart_pid_t pid)
{
	uart_port_t *port;

	uart_port_select(pid);
	port = uart_ports[uart_pid];
	BUG_ON(!port || !port->cleanup);
	port->cleanup();
#if 0
	bulk_clear_buffer(uart_states[uart_pid].bulk_out);
	bulk_clear_buffer(uart_states[uart_pid].bulk_in);
#endif
	clear_bit(pid, uart_port_regs);
}

int uart_put_char(uint8_t c)
{
	int ret = 0;

#if 0
	struct circ_buf *circ = &state->xmit
	if (uart_circ_chars_free(circ) != 0) {
		circ->buf[circ->head] = c;
		circ->head = (circ->head + 1) & (UART_XMIT_SIZE - 1);
		ret = 1;
	}
#endif
	return ret;
}

void uart_flush_chars(void)
{
	uart_start();
}

int uart_write(const uint8_t *buf, int count)
{
	uart_port_t *port = uart_ports[uart_pid];
	int c, ret = 0;

#if 0
	struct circ_buf *circ = &state->xmit;
	while (1) {
		c = CIRC_SPACE_TO_END(circ->head, circ->tail, UART_XMIT_SIZE);
		if (count < c)
			c = count;
		if (c <= 0)
			break;
		memcpy(circ->buf + circ->head, buf, c);
		circ->head = (circ->head + c) & (UART_XMIT_SIZE - 1);
		buf += c;
		count -= c;
		ret += c;
	}
#endif
	uart_start();

	return ret;
}

void uart_config_port(uint8_t params, uint32_t baudrate)
{
	uart_port_t *port = uart_ports[uart_pid];

	BUG_ON(!port || !port->set_params);
	port->config(params, baudrate);
}

uart_pid_t uart_register_port(uart_port_t *port)
{
	uart_pid_t pid = uart_nr_ports;

	BUG_ON(pid >= NR_UART_PORTS);
	uart_ports[pid] = port;
	uart_nr_ports++;

	return pid;
}

void uart_async_init(void)
{
}

