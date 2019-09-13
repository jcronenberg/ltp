// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

#include <errno.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#include <linux/capability.h>

static void test_setup(unsigned int);

static struct __user_cap_header_struct header;
static struct __user_cap_data_struct data;

struct test_case_t {
	cap_user_header_t headerp;
	cap_user_data_t datap;
	int exp_errno;
	char *errdesc;
} test_cases[] = {
#ifndef UCLINUX
	/* Skip since uClinux does not implement memory protection */
	{
	(cap_user_header_t) - 1, &data, EFAULT, "EFAULT"}, {
	&header, (cap_user_data_t) - 1, EFAULT, "EFAULT"},
#endif
	{
	&header, &data, EINVAL, "EINVAL"}, {
	&header, &data, EINVAL, "EINVAL"}, {
	&header, &data, ESRCH, "ESRCH"}
};


static void run(unsigned int n)
{
	test_setup(n);
	TEST(tst_syscall(__NR_capget, test_cases[n].headerp,
		     test_cases[n].datap));

	if (TST_RET == -1 &&
		TST_ERR == test_cases[n].exp_errno) {
			tst_res(TPASS | TTERRNO,
				"capget failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
			 "capget failed unexpectedly (%ld)", TST_RET);
	}
}

/*void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
}*/

void test_setup(unsigned int i)
{
#ifdef UCLINUX
	i = i + 2;
#endif
	switch (i) {

	case 0:
		break;
	case 1:
		header.version = _LINUX_CAPABILITY_VERSION;
		header.pid = getpid();
		break;
	case 2:
		header.version = 0;
		header.pid = getpid();
		break;
	case 3:
		header.version = _LINUX_CAPABILITY_VERSION;
		header.pid = -1;
		break;
	case 4:
		header.version = _LINUX_CAPABILITY_VERSION;
		header.pid = tst_get_unused_pid();
		break;
	}
}

static struct tst_test test = { 
        //.setup = setup,
        .test = run,
	.tcnt = sizeof(test_cases) / sizeof(test_cases[0]),
};
