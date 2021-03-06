#ifndef __IO_H_INCLUDE__
#define __IO_H_INCLUDE__

#include <target/types.h>

#define O_RDONLY	0x01
#define O_WRONLY	0x02
#define O_RDWR		0x03
#define O_DIRECT	0x08

/* Following flags are not used for the IO user, but for the IO channels.
 */
#define O_BUFFER	0x00
#define O_CMPL		0x80
#define O_AVAL		0x00
#define O_WRCMPL	(O_CMPL | O_WRONLY)
#define O_WRAVAL	(O_AVAL | O_WRONLY)
#define O_REAP		0x40
#define O_RDREAP	(O_AVAL | O_RDONLY | O_REAP)
#define O_RDAVAL	(O_AVAL | O_RDONLY)

typedef uint8_t (*iord_cb)(void);
typedef void (*iowr_cb)(uint8_t);
typedef void (*io_cb)(void);
typedef boolean (*iotest_cb)(void);
typedef void (*iordwr_cb)(uint8_t *c);
typedef boolean (*iobyte_cb)(uint8_t *c);

#endif /* __IO_H_INCLUDE__ */
