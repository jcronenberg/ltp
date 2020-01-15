// SPDX-License-Identifier: GPL-2.0
/*
 *  timerfd() test by Davide Libenzi (test app for timerfd)
 *  Copyright (C) 2007  Davide Libenzi
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 *  Description:
 *	Test timerfd with the flags:
 *		1) CLOCK_MONOTONIC
 *		2) CLOCK_REALTIME
 *
 * HISTORY
 *	28/05/2008 Initial contribution by Davide Libenzi <davidel@xmailserver.org>
 *      28/05/2008 Integrated to LTP by Subrata Modak <subrata@linux.vnet.ibm.com>
 */

#define _GNU_SOURCE
#include <poll.h>
#include "tst_test.h"
#include "lapi/timerfd.h"

static struct tcase {
	int id;
	char const *name;
} tcases[] = {
	{CLOCK_MONOTONIC, "CLOCK MONOTONIC"},
	{CLOCK_REALTIME, "CLOCK REALTIME"},
};

static unsigned long long getustime(int clockid)
{
	struct timespec tp;

	if (clock_gettime((clockid_t) clockid, &tp)) {
		tst_res(TFAIL | TERRNO, "clock_gettime() failed");
		return 0;
	}

	return 1000000ULL * tp.tv_sec + tp.tv_nsec / 1000;
}

static void set_timespec(struct timespec *tmr, unsigned long long ustime)
{
	tmr->tv_sec = (time_t) (ustime / 1000000ULL);
	tmr->tv_nsec = (long)(1000ULL * (ustime % 1000000ULL));
}

static void waittmr(int tfd, int timeo)
{
	u_int64_t ticks;
	struct pollfd pfd;

	pfd.fd = tfd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	if (poll(&pfd, 1, timeo) < 0) {
		tst_res(TFAIL | TERRNO, "poll() failed");
		return;
	}
	if ((pfd.revents & POLLIN) == 0) {
		tst_res(TFAIL, "no ticks happened");
		return;
	}
	SAFE_READ(0, tfd, &ticks, sizeof(ticks));

	if (ticks <= 0)
		tst_res(TFAIL, "got no timer");

	return;
}

static void run(unsigned int n)
{
	int tfd;
	unsigned long long tnow;
	u_int64_t uticks;
	struct itimerspec tmr;
	struct tcase *clks = &tcases[n];

	if (TFD_TIMER_ABSTIME == 0)
		tst_brk(TCONF, "Test not suited for this system");

	tst_res(TINFO, "testing %s", clks->name);

	tfd = timerfd_create(clks->id, 0);
	if (tfd == -1) {
		tst_res(TFAIL | TERRNO, "timerfd_create() failed");
		return;
	}

	tnow = getustime(clks->id);
	set_timespec(&tmr.it_value, 500 * 1000);
	set_timespec(&tmr.it_interval, 0);
	if (timerfd_settime(tfd, 0, &tmr, NULL)) {
		tst_res(TFAIL | TERRNO, "timerfd_settime");
		return;
	}
	waittmr(tfd, -1);

	tnow = getustime(clks->id);
	set_timespec(&tmr.it_value, tnow + 500 * 1000);
	set_timespec(&tmr.it_interval, 0);
	if (timerfd_settime(tfd, TFD_TIMER_ABSTIME, &tmr, NULL)) {
		tst_res(TFAIL | TERRNO, "timerfd_settime failed");
		return;
	}
	waittmr(tfd, -1);

	tnow = getustime(clks->id);
	set_timespec(&tmr.it_value, tnow + 100 * 1000);
	set_timespec(&tmr.it_interval, 100 * 1000);
	if (timerfd_settime(tfd, TFD_TIMER_ABSTIME, &tmr, NULL)) {
		tst_res(TFAIL | TERRNO, "timerfd_settime failed");
		return;
	}

	if (timerfd_gettime(tfd, &tmr)) {
		tst_res(TFAIL | TERRNO, "timerfd_gettime failed");
		return;
	}
	waittmr(tfd, -1);

	tst_res(TINFO, "testing with O_NONBLOCK");
	tnow = getustime(clks->id);
	set_timespec(&tmr.it_value, 100 * 1000);
	set_timespec(&tmr.it_interval, 0);
	if (timerfd_settime(tfd, 0, &tmr, NULL)) {
		tst_res(TFAIL | TERRNO, "timerfd_settime failed");
		return;
	}
	waittmr(tfd, -1);

	SAFE_FCNTL(tfd, F_SETFL, fcntl(tfd, F_GETFL, 0) | O_NONBLOCK);

	TEST(read(tfd, &uticks, sizeof(uticks)));
	if (TST_RET > 0)
		tst_res(TFAIL, "timer ticks not zero");
	else if (TST_ERR != EAGAIN)
		tst_res(TFAIL | TERRNO, "expected errno EAGAIN got");
	else
		tst_res(TPASS, "Passed test %s", clks->name);

	SAFE_FCNTL(tfd, F_SETFL, fcntl(tfd, F_GETFL, 0) & ~O_NONBLOCK);
	SAFE_CLOSE(tfd);

}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.min_kver = "2.6.25",
};
