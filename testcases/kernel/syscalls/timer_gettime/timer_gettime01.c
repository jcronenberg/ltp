/******************************************************************************
 * Copyright (c) Crackerjack Project., 2007                                   *
 * Porting from Crackerjack to LTP is done by:                                *
 *              Manas Kumar Nayak <maknayak@in.ibm.com>                       *
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>                          *
 *                                                                            *
 * This program is free software;  you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  *
 * the GNU General Public License for more details.                           *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program;  if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           *
 *                                                                            *
 ******************************************************************************/

#include <time.h>
#include <signal.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

/*static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();
}*/
/*
 * TODO
 * remove for loop
 * instead put timer and spec into a nice struct
 * prob add a replace into the struct aswell
 * not necessary but more elegant
 * more futurproof fingers crossed
 */

struct test_case {
	//int create_timer;
	char desc[100];
	int exp_err;
	int timer;
	struct itimerspec *spec;
};

struct test_case tc[] = {
	{
	 //.create_timer = 1,
	 .desc = "timer_gettime(CLOCK_REALTIME)",
	 .timer = 0,
	 .exp_err = 0,
	 },
	{
	 .desc = "timer_gettime(minusone)",
	 .timer = -1,
	 .exp_err = EINVAL,
	 },
	{
	 //.create_timer = 1,
	 .desc = "timer_gettime(NULL)",
	 .timer = 0,
	 .exp_err = EFAULT,	 
	 .spec = NULL,
	 },
};

static int create_timer = 0;

static void run(unsigned int n)
{
	struct sigevent ev;
	
	create_timer++;

	ev.sigev_value = (union sigval) 0;
	ev.sigev_signo = SIGALRM;
	ev.sigev_notify = SIGEV_SIGNAL;
	if(create_timer == 1) {
		TEST(tst_syscall(__NR_timer_create, CLOCK_REALTIME, &ev, &tc[n].timer));

		printf ("Created Timer");

		if (TST_RET != 0)
			tst_brk(TBROK | TERRNO, "Failed to create timer");
	} 
	
	printf("timerid:%i", tc[n].timer);

	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(tc); i++) {
	TEST(tst_syscall(__NR_timer_gettime, tc[i].timer, &tc[i].spec));
	
	//printf("\nTST_RET = %li \nTST_ERR = %i\nDesc = %s", TST_RET, TST_ERR, tc[n].desc);	
	if (TST_RET == tc[i].exp_err) {
		tst_res(TPASS, "%s Passed", tc[i].desc);
	} else if (TST_RET == -1 && TST_ERR == tc[i].exp_err) {
		tst_res(TPASS,  "%s failed expectedly", tc[i].desc);
	} else if (TST_RET == 0)  {
		tst_res(TFAIL | TERRNO, "%s succeded unexpectedly", tc[i].desc);
	} else {
		tst_res(TFAIL | TERRNO, "%s failed", tc[i].desc);
	}
	}
}

static struct tst_test test = {
	//.setup = setup,
	.test = run,
	.needs_tmpdir = 1,
	.tcnt = 1,
};
