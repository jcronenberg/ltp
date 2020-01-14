/*
 *  timerfd() test by Davide Libenzi (test app for timerfd)
 *  Copyright (C) 2007  Davide Libenzi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 *
 *     $ gcc -o timerfd-test2 timerfd-test2.c -lrt
 *
 * NAME
 *	timerfd01.c
 * HISTORY
 *	28/05/2008 Initial contribution by Davide Libenzi <davidel@xmailserver.org>
 *      28/05/2008 Integrated to LTP by Subrata Modak <subrata@linux.vnet.ibm.com>
 */

#define _GNU_SOURCE
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include "tst_test.h"
#include <lapi/fcntl.h>
#include "lapi/syscalls.h"
#include "lapi/timerfd.h"
//#include "safe_macros.h"

/*
 * This were good at the time of 2.6.23-rc7 ...
 *
 * #ifdef __NR_timerfd
 *
 * ... but is not now with 2.6.25
 */
//#ifdef __NR_timerfd_create

/* Definitions from include/linux/timerfd.h */
//#define TFD_TIMER_ABSTIME (1 << 0)
//#else
//#define TFD_TIMER_ABSTIME (0)
//#endif

struct tmr_type {
	int id;
	char const *name;
};

static unsigned long long getustime(int clockid)
{
	struct timespec tp;

	if (clock_gettime((clockid_t) clockid, &tp)) {
		tst_res(TFAIL, "clock_gettime");
		return 0;
	}

	return 1000000ULL * tp.tv_sec + tp.tv_nsec / 1000;
}

static void set_timespec(struct timespec *tmr, unsigned long long ustime)
{
	tmr->tv_sec = (time_t) (ustime / 1000000ULL);
	tmr->tv_nsec = (long)(1000ULL * (ustime % 1000000ULL));
}

//static int timerfd_create(int clockid, int flags)
//{
//	return tst_syscall(__NR_timerfd_create, clockid, flags);
//}
//
//static int timerfd_settime(int ufc, int flags, const struct itimerspec *utmr,
//		    struct itimerspec *otmr)
//{
//	return tst_syscall(__NR_timerfd_settime, ufc, flags, utmr, otmr);
//}
//
//static int timerfd_gettime(int ufc, struct itimerspec *otmr)
//{
//	return tst_syscall(__NR_timerfd_gettime, ufc, otmr);
//}

static long waittmr(int tfd, int timeo)
{
	u_int64_t ticks;
	struct pollfd pfd;

	pfd.fd = tfd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	if (poll(&pfd, 1, timeo) < 0) {
		tst_res(TFAIL, "poll");
		return -1;
	}
	if ((pfd.revents & POLLIN) == 0) {
		tst_res(TFAIL, "no ticks happened");
		return -1;
	}
	SAFE_READ(0, tfd, &ticks, sizeof(ticks));
	//if (read(tfd, &ticks, sizeof(ticks)) != sizeof(ticks)) {
	//	tst_res(TFAIL, "reading timerfd failed");
	//	return -1;
	//}

	return ticks;
}

static void run(unsigned int n)
{
	int tfd;
	unsigned int i;
	long ticks;
	unsigned long long tnow; //ttmr;
	u_int64_t uticks;
	struct itimerspec tmr;
	struct tmr_type clks[] = {
		{CLOCK_MONOTONIC, "CLOCK MONOTONIC"},
		{CLOCK_REALTIME, "CLOCK REALTIME"},
	};

	if(TFD_TIMER_ABSTIME == 0)
		tst_brk(TCONF, "Test not suited for this system");

	for (i = 0; i < sizeof(clks) / sizeof(clks[0]); i++) {
		tst_res(TINFO, "testing %s", clks[i].name);

		set_timespec(&tmr.it_value, 500 * 1000);
		set_timespec(&tmr.it_interval, 0);
		tnow = getustime(clks[i].id);
		if ((tfd = timerfd_create(clks[i].id, 0)) == -1) {
			tst_res(TFAIL, "timerfd");
			return;
		}

		if (timerfd_settime(tfd, 0, &tmr, NULL)) {
			tst_res(TFAIL, "timerfd_settime");
			return;
		}

		ticks = waittmr(tfd, -1);
		//ttmr = getustime(clks[i].id);
		if (ticks <= 0)
			tst_res(TFAIL, "got no timer");

		tnow = getustime(clks[i].id);
		set_timespec(&tmr.it_value, tnow + 500 * 1000);
		set_timespec(&tmr.it_interval, 0);
		if (timerfd_settime(tfd, TFD_TIMER_ABSTIME, &tmr, NULL)) {
			tst_res(TFAIL, "timerfd_settime failed");
			return;
		}

		ticks = waittmr(tfd, -1);
		//ttmr = getustime(clks[i].id);
		if (ticks <= 0)
			tst_res(TFAIL, "got no timer");

		tnow = getustime(clks[i].id);
		set_timespec(&tmr.it_value, tnow + 100 * 1000);
		set_timespec(&tmr.it_interval, 100 * 1000);
		if (timerfd_settime(tfd, TFD_TIMER_ABSTIME, &tmr, NULL)) {
			tst_res(TFAIL, "timerfd_settime failed");
			return;
		}

		tst_res(TINFO, "Sleeping for 1 second");
		sleep(1);
		if (timerfd_gettime(tfd, &tmr)) {
			tst_res(TFAIL, "timerfd_gettime failed");
			return;
		}
		tst_res(TINFO, "timerfd_gettime returned:\n"
			"it_value = { %ld, %ld } it_interval = { %ld, %ld }",
			(long)tmr.it_value.tv_sec, (long)tmr.it_value.tv_nsec,
			(long)tmr.it_interval.tv_sec,
			(long)tmr.it_interval.tv_nsec);
		tst_res(TINFO, "Sleeping for 1 second");
		sleep(1);

		ticks = waittmr(tfd, -1);
		//ttmr = getustime(clks[i].id);
		if (ticks <= 0)
			tst_res(TFAIL, "got no timer");

		tst_res(TINFO, "testing O_NONBLOCK");
		tnow = getustime(clks[i].id);
		set_timespec(&tmr.it_value, 100 * 1000);
		set_timespec(&tmr.it_interval, 0);
		if (timerfd_settime(tfd, 0, &tmr, NULL)) {
			tst_res(TFAIL, "timerfd_settime failed");
			return;
		}

		ticks = waittmr(tfd, -1);
		//ttmr = getustime(clks[i].id);
		if (ticks <= 0)
			tst_res(TFAIL, "got no timer");

		SAFE_FCNTL(tfd, F_SETFL, fcntl(tfd, F_GETFL, 0) | O_NONBLOCK);

		TEST(read(tfd, &uticks, sizeof(uticks)));
		if(TST_RET > 0)
			tst_res(TFAIL, "timer ticks not zero");
		else if (TST_ERR != EAGAIN)
			tst_res(TFAIL | TERRNO, "expected errno EAGAIN got");
		else
			tst_res(TPASS, "Passed test %s %i",clks[i].name , n);

		SAFE_FCNTL(tfd, F_SETFL, fcntl(tfd, F_GETFL, 0) & ~O_NONBLOCK);

		SAFE_CLOSE(tfd);
	}
}

static struct tst_test test = {
	.test = run,
	.tcnt = 3,
	.min_kver = "2.6.25",
};
