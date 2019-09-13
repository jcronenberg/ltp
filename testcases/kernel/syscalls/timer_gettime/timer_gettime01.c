// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Porting from Crackerjack to LTP is done by:
 *              Manas Kumar Nayak <maknayak@in.ibm.com>
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 */

#include <time.h>
#include <signal.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

static void gettime(void)
{
	struct sigevent ev;
	struct itimerspec spec;
	int timer;

	ev.sigev_value = (union sigval) 0;
	ev.sigev_signo = SIGALRM;
	ev.sigev_notify = SIGEV_SIGNAL;
	TEST(tst_syscall(__NR_timer_create, CLOCK_REALTIME, &ev, &timer));

	if (TST_RET != 0)
		tst_brk(TBROK | TERRNO, "Failed to create timer");

	TEST(tst_syscall(__NR_timer_gettime, timer, &spec));
	if (TST_RET == 0) {
		tst_res(TPASS, "timer_gettime(CLOCK_REALTIME) Passed");
	} else {
		tst_res(TFAIL | TERRNO,
			"timer_gettime(CLOCK_REALTIME) Failed");
	}

	TEST(tst_syscall(__NR_timer_gettime, -1, &spec));
	if (TST_RET == -1 && TST_ERR == EINVAL) {
		tst_res(TPASS,	"timer_gettime(-1) Failed: EINVAL");
	} else {
		tst_res(TFAIL | TERRNO,
			"timer_gettime(-1) = %li", TST_RET);
	}

	TEST(tst_syscall(__NR_timer_gettime, timer, NULL));
	if (TST_RET == -1 && TST_ERR == EFAULT) {
		tst_res(TPASS,	"timer_gettime(NULL) Failed: EFAULT");
	} else {
		tst_res(TFAIL | TERRNO,
			"timer_gettime(-1) = %li", TST_RET);
	}
}

static struct tst_test test = {
	.test_all = gettime,
};
