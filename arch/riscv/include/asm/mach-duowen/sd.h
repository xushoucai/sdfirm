/*
 * ZETALOG's Personal COPYRIGHT
 *
 * Copyright (c) 2019
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
 * @(#)sd.h: duowen specific secure digital controller interface
 * $Id: sd.h,v 1.1 2019-10-09 15:47:00 zhenglv Exp $
 */

#ifndef __SD_DUOWEN_H_INCLUDE__
#define __SD_DUOWEN_H_INCLUDE__

#define SDHC_REG(n, offset)	(SD_BASE + (offset))
#define SD_FREQ_MIN	25000000
#define SD_FREQ_MAX	25000000

#define SD_CLASS2	1
#define SD_CLASS5	1
#define SD_CLASS8	1
#define SD_CLASS10	1

#ifdef CONFIG_DUOWEN_SD
#include <driver/sdhci.h>
#ifndef ARCH_HAVE_SD
#define ARCH_HAVE_SD		1
#else
#error "Multiple SD controller defined"
#endif
#endif

#define mmc_hw_ctrl_init()		sdhci_init(SD_FREQ_MIN, SD_FREQ_MAX)
#define mmc_hw_slot_select(sid)		do { } while (0)
#define mmc_hw_card_detect()		sdhci_detect_card()
#define mmc_hw_set_clock(clock)		sdhci_set_clock(clock)
#define mmc_hw_set_width(width)		sdhci_set_width(width)
#define mmc_hw_card_busy()		sdhci_card_busy()
#define mmc_hw_send_command(cmd, arg)	sdhci_send_command(cmd, arg)
#define mmc_hw_recv_response(resp, size)	\
	sdhci_recv_response(resp, size)
#define mmc_hw_tran_data(dat, len, cnt)	sdhci_tran_data(dat, len, cnt)
#define mmc_hw_irq_init()		sdhci_irq_init()
#define mmc_hw_irq_poll()		sdhci_irq_poll()

#endif /* __SD_DUOWEN_H_INCLUDE__ */
