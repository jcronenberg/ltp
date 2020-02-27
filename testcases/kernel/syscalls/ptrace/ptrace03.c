// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2019 Jorik Cronenberg <jcronenberg@suse.de>
 *
 * Author: Saji Kumar.V.R <saji.kumar@wipro.com>
 * Ported to new library: Jorik Cronenberg <jcronenberg@suse.de>
 *
 *This test verifies that
 *1) ptrace() returns -1 & sets errno to EPERM while tring to trace
 *   process 1
 *   (This test case will be executed only if the kernel version
 *   is 2.6.25 or below)
 *2) ptrace() returns -1 & sets errno to ESRCH if process with
 *   specified pid does not exist
 *3) ptrace() returns -1 & sets errno to EPERM if we are trying
 *   to trace a process which is already been traced
 */

#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>
#include <config.h>
#include "ptrace.h"
#include "tst_test.h"

static pid_t init_pid = 1;
static pid_t unused_pid;
static pid_t zero_pid;

struct test_case_t {
	int request;
	pid_t *pid;
	int exp_errno;
} test_cases[] = {
	{PTRACE_ATTACH, &init_pid, EPERM},
	{PTRACE_ATTACH, &unused_pid, ESRCH},
	{PTRACE_TRACEME, &zero_pid, EPERM},
};

static void setup(void)
{
	unused_pid = tst_get_unused_pid();
}

static void run(unsigned int i)
{
	pid_t child_pid;
	int status;

	if (i == 0 && tst_kvercmp(2, 6, 25) > 0) {
		tst_res(TCONF, "this kernel allows to trace init");
		return;
	}

	child_pid = SAFE_FORK();

	if (!child_pid) {
		if (i == 2) {
			if ((ptrace(PTRACE_TRACEME, 0, NULL, NULL)) == -1) {
				tst_res(TWARN | TERRNO, "ptrace() falied");
				exit(0);
			}
		}

		TEST(ptrace(test_cases[i].request,
			    *(test_cases[i].pid), NULL, NULL));
		if ((TST_RET == -1) && (TST_ERR == test_cases[i].exp_errno))
			exit(TST_ERR);
		else {
			tst_res(TWARN | TERRNO,
				 "ptrace() returned %ld", TST_RET);
			exit(TST_ERR);
		}
	} else {
		SAFE_WAITPID(child_pid, &status, 0);

		if ((WIFEXITED(status)) &&
			  (WEXITSTATUS(status) == test_cases[i].exp_errno)) {
			tst_res(TPASS,
				"ptrace() returned errno: %i as expected",
				test_cases[i].exp_errno);
		} else {
			tst_res(TFAIL,
				"ptrace() returned errno: %i, expected: %i",
				WEXITSTATUS(status), test_cases[i].exp_errno);
		}
	}
}


static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(test_cases),
	.forks_child = 1,
};
