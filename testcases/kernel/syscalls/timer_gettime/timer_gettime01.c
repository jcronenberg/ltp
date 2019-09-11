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

static void get_time(void)
{
	int lc;

	struct sigevent ev;
	struct itimerspec spec;
	int timer;

	//tst_parse_opts(ac, av, NULL, NULL);

	ev.sigev_value = (union sigval) 0;
	ev.sigev_signo = SIGALRM;
	ev.sigev_notify = SIGEV_SIGNAL;
	TEST(tst_syscall(__NR_timer_create, CLOCK_REALTIME, &ev, &timer));

	if (TST_RET != 0)
		tst_brk(TBROK | TERRNO, "Failed to create timer");

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		//tst_count = 0;

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
}

static struct tst_test test = {
	//.setup = setup,
	.test_all = get_time,
	.needs_tmpdir = 1
};
