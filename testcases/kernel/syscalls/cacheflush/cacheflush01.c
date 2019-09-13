// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Maxin John <maxin.john@gmail.com>, 2009                      */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

#if __NR_cacheflush != __LTP__NR_INVALID_SYSCALL
#include <asm/cachectl.h>
#else
#ifndef   ICACHE
#define   ICACHE   (1<<0)	/* flush instruction cache        */
#endif
#ifndef   DCACHE
#define   DCACHE   (1<<1)	/* writeback and flush data cache */
#endif
#ifndef   BCACHE
#define   BCACHE   (ICACHE|DCACHE)	/* flush both caches              */
#endif
#endif

static void run(void)
{

	char *addr = NULL;



	/* Create some user address range */
	addr = malloc(getpagesize());
	if (addr == NULL) {
		tst_brk(TFAIL | TTERRNO, "malloc failed");
	}

	/* Invokes cacheflush() with proper parameters */
	TEST(tst_syscall(__NR_cacheflush, addr, getpagesize(), ICACHE));
	if (TST_RET == 0) {
		tst_res(TPASS, "passed with no errno");
	} else {
		tst_res(TFAIL, "failed with unexpected errno");
	}

	TEST(tst_syscall(__NR_cacheflush, addr, getpagesize(), DCACHE));
	if (TST_RET == 0) {
		tst_res(TPASS, "passed with no errno");
	} else {
		tst_res(TFAIL, "failed with unexpected errno");
	}

	TEST(tst_syscall(__NR_cacheflush, addr, getpagesize(), BCACHE));
	if (TST_RET == 0) {
		tst_res(TPASS, "passed with no errno");
	} else {
		tst_res(TFAIL, "failed with unexpected errno");
	}

}

static struct tst_test test = {
	.test_all = run,
};
