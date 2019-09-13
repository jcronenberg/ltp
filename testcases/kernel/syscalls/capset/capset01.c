// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * CHANGES:
 *  2005/01/01: add a hint to a possible solution when test fails
 *              - Ricky Ng-Adam <rngadam@yahoo.com>
 */
#include <unistd.h>
#include <errno.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#include <linux/capability.h>

static struct __user_cap_header_struct header;
	/* cap_user_header_t is a pointer
	 * to __user_cap_header_struct
	 */

static struct __user_cap_data_struct data;
	/* cap_user_data_t is a pointer to
	 * __user_cap_data_struct
	 */

static void run(void)
{
	TEST(tst_syscall(__NR_capset, &header, &data));

	if (TST_RET == 0) {
		tst_res(TPASS, "capset() returned %ld", TST_RET);
	} else {
		tst_res(TFAIL | TTERRNO,
			 "Test Failed, capset() returned %ld\nMaybe you need to do `modprobe capability`?",
			 TST_RET);
	}
}

void setup(void)
{
	header.version = _LINUX_CAPABILITY_VERSION;
	header.pid = 0;
	if (tst_syscall(__NR_capget, &header, &data) == -1)
		tst_brk(TBROK | TERRNO, "capget() failed");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
};
