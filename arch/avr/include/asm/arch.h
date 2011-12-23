#ifndef __ARCH_AVR_H_INCLUDE__
#define __ARCH_AVR_H_INCLUDE__

#include <target/config.h>
#include <asm/mach/arch.h>

#define nop()		__asm__ __volatile__ ("nop" ::)
#define waiti()		__asm__ __volatile__ ("sleep" ::)

#endif /* __ARCH_AVR_H_INCLUDE__ */