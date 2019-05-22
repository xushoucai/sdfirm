#ifndef __COMPILER_GCC_H_INCLUDE__
#define __COMPILER_GCC_H_INCLUDE__

#ifdef __CHECKER__
# define __safe		__attribute__((safe))
# define __force	__attribute__((force))
# define __nocast	__attribute__((nocast))
# define __iomem	__attribute__((noderef, address_space(2)))
#else
# define __safe
# define __force
# define __nocast
# define __iomem
#endif

#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#  define __INTR_ATTRS used, externally_visible
#else /* GCC < 4.1 */
#  define __INTR_ATTRS used
#endif

#ifdef CONFIG_CC_ISR_VECTOR
#define DEFINE_ISR(vector, ...)	\
	void vector(void) __attribute__ ((signal,__INTR_ATTRS)) __VA_ARGS__;\
	void vector(void)
#define _VECTOR(N) __vector_ ## N
#endif
#define ISR_BLOCK
#define ISR_NOBLOCK    __attribute__((interrupt))
#define ISR_NAKED      __attribute__((naked))
#define ISR_ALIASOF(v) __attribute__((alias(__STRINGIFY(v))))

#ifdef __ASSEMBLY__
#define ULL(x)		x
#define UL(x)		x
#else
#define ULL(x)		x##ULL
#define UL(x)		x##UL
#endif

#define __packed			__attribute__((packed))
#define __used				__attribute__((used))
#define __unused			__attribute__((unused))
#define __align(a)			__attribute__((__aligned__(a)))
#define __always_inline			inline __attribute__((always_inline))
#define __compiler_offsetof(a, b)	 __builtin_offsetof(a, b)
#define __section(S)			__attribute__((__section__(#S)))

#define __stringify_1(x...)	#x
#define __stringify(x...)	__stringify_1(x)

#endif  /* __COMPILER_GCC_H_INCLUDE__ */
