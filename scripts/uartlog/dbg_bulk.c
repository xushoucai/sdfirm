#include <host/uartdbg.h>

static void bulk_dump_set_flag(void *ctx, dbg_cmd_t cmd, dbg_data_t data)
{
	switch (data) {
	case 0x01:
		dbg_dumper(ctx, cmd, "+SOFT");
		break;
	case 0x02:
		dbg_dumper(ctx, cmd, "+HARD");
		break;
	case 0x04:
		dbg_dumper(ctx, cmd, "+BACK");
		break;
	case 0x08:
		dbg_dumper(ctx, cmd, "+SYNC");
		break;
	case 0x40:
		dbg_dumper(ctx, cmd, "+BUSY");
		break;
	case 0x80:
		dbg_dumper(ctx, cmd, "+HALT");
		break;
	default:
		dbg_dumper(ctx, cmd, "BULK_FLAG+%02x", data);
		break;
	}
}

static void bulk_dump_clear_flag(void *ctx, dbg_cmd_t cmd, dbg_data_t data)
{
	switch (data) {
	case 0x01:
		dbg_dumper(ctx, cmd, "-SOFT");
		break;
	case 0x02:
		dbg_dumper(ctx, cmd, "-HARD");
		break;
	case 0x04:
		dbg_dumper(ctx, cmd, "-BACK");
		break;
	case 0x08:
		dbg_dumper(ctx, cmd, "-SYNC");
		break;
	case 0x40:
		dbg_dumper(ctx, cmd, "-BUSY");
		break;
	case 0x80:
		dbg_dumper(ctx, cmd, "-HALT");
		break;
	default:
		dbg_dumper(ctx, cmd, "BULK_FLAG-%02x", data);
		break;
	}
}

static void bulk_dump_flow(void *ctx, dbg_cmd_t cmd, dbg_data_t data)
{
	switch (data) {
	case 0x00:
		dbg_dumper(ctx, cmd, "flow=OPEN");
		break;
	case 0x01:
		dbg_dumper(ctx, cmd, "flow=CLOSE");
		break;
	case 0x02:
		dbg_dumper(ctx, cmd, "flow=START");
		break;
	case 0x03:
		dbg_dumper(ctx, cmd, "flow=STOP");
		break;
	case 0x04:
		dbg_dumper(ctx, cmd, "flow=HALT");
		break;
	case 0x05:
		dbg_dumper(ctx, cmd, "flow=UNHALT");
		break;
	case 0x06:
		dbg_dumper(ctx, cmd, "flow=BH");
		break;
	default:
		dbg_dumper(ctx, cmd, "flow=%02x", data);
		break;
	}
}

static void bulk_dump_channel(void *ctx, dbg_cmd_t cmd, dbg_data_t data)
{
	dbg_dumper(ctx, cmd, "cid=%02x", data);
}

struct dbg_parser dbg_bulk_events[NR_BULK_EVENTS] = {
	{ "FLAG", 0, bulk_dump_set_flag, },
	{ "FLAG", 0, bulk_dump_clear_flag, },
	{ "FLOW", 0, bulk_dump_flow, },
	{ "CHAN", 0, bulk_dump_channel, },
};

struct dbg_source dbg_bulk_state = {
	"bulk",
	dbg_bulk_events,
	NR_BULK_EVENTS,
};

void dbg_bulk_init(void)
{
	dbg_register_source(DBG_SRC_BULK, &dbg_bulk_state);
}
