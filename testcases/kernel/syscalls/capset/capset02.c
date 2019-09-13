// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#include <linux/capability.h>

#define INVALID_VERSION 0

static void test_setup(unsigned int);
static void child_func(void);

static pid_t child_pid = -1;


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
	(cap_user_header_t) -1, &data, EFAULT, "EFAULT"}, {
	&header, (cap_user_data_t) -1, EFAULT, "EFAULT"},
#endif
	{
	&header, &data, EINVAL, "EINVAL"}, {
	&header, &data, EPERM, "EPERM"},
};


static void run(unsigned int i)
{

#ifdef UCLINUX
	maybe_run_child(&child_func, "");
#endif


#ifdef UCLINUX
	i = 2;
#else
	i = 0;
#endif

	test_setup(i);
	TEST(tst_syscall(__NR_capset, test_cases[i].headerp,
		     test_cases[i].datap));

	if (TST_RET == -1 && TST_ERR == test_cases[i].exp_errno) {
		tst_res(TPASS, "capset() returned -1,\nerrno: %s",
			       test_cases[i].errdesc);
	} else {
		tst_res(TFAIL | TTERRNO,
			"Test Failed, capset() returned %ld", TST_RET);
	}
}

void setup(void)
{
	/*
	 * Save current capability data.
	 * header.version must be _LINUX_CAPABILITY_VERSION
	 */
	header.version = _LINUX_CAPABILITY_VERSION;
	if (tst_syscall(__NR_capget, &header, &data) == -1)
		tst_brk(TBROK | TERRNO, "capget failed");
}

void cleanup(void)
{
	if (child_pid > 0) {
		kill(child_pid, SIGTERM);
		wait(NULL);
	}
}

void child_func(void)
{
	for (;;)
		sleep(10);
}

void test_setup(unsigned int i)
{
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;

	switch (i) {
	case 0:
		break;

	case 1:
		header.version = _LINUX_CAPABILITY_VERSION;
		header.pid = 0;
		break;

	case 2:
		header.version = INVALID_VERSION;
		header.pid = 0;
		break;

	case 3:
		header.version = _LINUX_CAPABILITY_VERSION;
		/*
		 * when a non-zero pid is specified, process should have
		 * CAP_SETPCAP capability to change capabilities.
		 * by default, CAP_SETPCAP is not enabled. So giving
		 * a non-zero pid results in capset() failing with
		 * errno EPERM
		 *
		 * Note: this seems to have changed with recent kernels
		 * => create a child and try to set its capabilities
		 */
		child_pid = SAFE_FORK();
		if (child_pid == -1)
			tst_brk(TBROK | TERRNO, "fork failed");
		else if (child_pid == 0) {
#ifdef UCLINUX
			if (self_exec(argv0, "") < 0) {
				perror("self_exec failed");
				exit(1);
			}
#else
			child_func();
#endif
		} else {
			header.pid = child_pid;
			ltpuser = getpwnam(nobody_uid);
			if (ltpuser == NULL)
				tst_brk(TBROK | TERRNO, "getpwnam failed");
			SAFE_SETEUID(ltpuser->pw_uid);

		}
		break;

	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
	.tcnt = ARRAY_SIZE(test_cases) / sizeof(test_cases[0]),
	.needs_root = 1,
};
