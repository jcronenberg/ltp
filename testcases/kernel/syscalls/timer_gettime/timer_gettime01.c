//SPDX-License-Identifier: GPL-2.0-or-later
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

static struct tcase {
	int exp_err;
	int timer;
	struct itimerspec spec;
	char const *name;
} tcases[] = {
	{
	 .exp_err = 0,
	 .timer = 0,
	 .name = "timer_gettime(CLOCK_REALTIME)",
	 },
	{
	 .exp_err = EINVAL,
	 .timer = -1,
	 .name = "timer_gettime(minusone)",
	 },
	{
	 .exp_err = EFAULT,	 
	 .timer = 0,
	 .spec = ,
	 .name = "timer_gettime(NULL)",
	 },
};

static void run(unsigned int n)
{
	struct sigevent ev;
	struct tcase *tc = &tcases[n];
	
	ev.sigev_value = (union sigval) 0;
	ev.sigev_signo = SIGALRM;
	ev.sigev_notify = SIGEV_SIGNAL;
	if(!n) {
		TEST(tst_syscall(__NR_timer_create, CLOCK_REALTIME, &ev, &tc->timer));
		if (TST_RET != 0)
			tst_brk(TBROK | TERRNO, "Failed to create timer");
	} 

	TEST(tst_syscall(__NR_timer_gettime, tc->timer, &tc->spec));
	
		if (TST_RET == tc->exp_err)
			tst_res(TPASS, "%s Passed", tc->name);
		else if (TST_RET == -1 && TST_ERR == tc->exp_err)
			tst_res(TPASS,  "%s failed expectedly", tc->name);
		else if (TST_RET == 0)
			tst_res(TFAIL, "%s succeded unexpectedly", tc->name);
		else
			tst_res(TFAIL | TERRNO, "%s failed", tc->name);

}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
