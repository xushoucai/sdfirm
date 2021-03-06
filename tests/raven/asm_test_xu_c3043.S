// ------------------------------------------------------------
// ARMv8-A separate core program
//
// Description: Test case
// ------------------------------------------------------------

#ifdef TESTOS
#include <target/raven.h>
ENTRY(asm_test_xu_c3043)
#else
    .global asm_test_xu_c3043
    .type asm_test_xu_c3043, "function"
    .cfi_startproc
asm_test_xu_c3043:
#endif
    nop
    mov w9, 0

    // add your core code
    movi v0.4s, #0
    movi v1.4s, #0
    movi v2.4s, #0
    movi v3.4s, #0
    movi v29.4s, #0
    movi v30.4s, #0
    orr v0.4s, #0x3d
    orr v1.4s, #0x41
    orr v2.4s, #0xaf
    orr v3.4s, #0xf2
    orr v4.4s, #0x92
    pmul v5.16b, v2.16b, v1.16b
    pmull2 v6.8h, v5.16b, v3.16b
    and v30.16b, v30.16b, v29.16b
    orr v30.4s, #0x6f
    cmeq v30.4s, v5.4s, v30.4s
    fmov x1, d30
    cbz x1, _skip_pass
    // end of add your code

    mov w9, 1
_skip_pass:
    // write to address 44'h0FF_7003_FFC0 if your program pass; if fail, write 0
#ifdef TESTOS
    mov w0, w9
#else
#ifdef RTL_SIM
    movk x8, #0xFFC0
    movk x8, #0x7003, lsl #16
    movk x8, #0x00FF, lsl #32
    movk x8, #0x0000, lsl #48
#else
    movk x8, #0xFFC0
    movk x8, #0x8003, lsl #16
    movk x8, #0x0000, lsl #32
    movk x8, #0x0000, lsl #48
#endif
    str x30, [sp, #-0x8]	// x30 is lr
    bl GetCPUID			// 0-47
    add x8, x8, x0
    strb w9, [x8, #0]
    nop
    dsb ish
    ldr x30, [sp, #-0x8]
#endif
    ret
#ifdef TESTOS
ENDPROC(asm_test_xu_c3043)
define_asm_testfn asm_test_xu_c3043 0
#else
    .cfi_endproc
#endif
