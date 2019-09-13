// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

#include <errno.h>
#include <unistd.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#include <linux/capability.h>



static struct __user_cap_header_struct header;	/* cap_user_header_t is a pointer
						   to __user_cap_header_struct */

static struct __user_cap_data_struct data;	/* cap_user_data_t is a pointer to
						   __user_cap_data_struct */

static void run(void)
{

	header.version = _LINUX_CAPABILITY_VERSION;

	TEST(tst_syscall(__NR_capget, &header, &data));

	if (TST_RET == 0) {
		tst_res(TPASS, "capget() returned %ld", TST_RET);
	} else {
		tst_res(TFAIL | TTERRNO,
			 "Test Failed, capget() returned %ld",
			 TST_RET);
	}
}

/*void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
}*/

static struct tst_test test = {
	//.setup = setup,
	.test_all = run,
};
