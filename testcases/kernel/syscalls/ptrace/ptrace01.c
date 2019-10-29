// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * Author:	Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 * Ported to new library:
 * 10/2019	Jorik Cronenberg <jcronenberg@suse.de>
 *
 * DESCRIPTION
 *	This test case tests the functionality of ptrace() for
 *	PTRACE_TRACEME & PTRACE_KILL requests.
 *	Here, we fork a child & the child does ptrace(PTRACE_TRACEME, ...).
 *	Then a signal is delivered to the child & verified that parent
 *	is notified via wait(). then parent does ptrace(PTRACE_KILL, ..)
 *	to kill the child. Again parent wait() for child to finish.
 *	If child finished abnormally, test passes.
 *		We test two cases
 *			1) By telling child to ignore SIGUSR2 signal
 *			2) By installing a signal handler for child for SIGUSR2
 *		In both cases, child should stop & notify parent on reception
 *		of SIGUSR2
 *
 */

#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <config.h>
#include "ptrace.h"
#include "tst_test.h"

static volatile int got_signal;

static void child_handler(void)
{
	SAFE_KILL(getppid(), SIGUSR2);
}

static void parent_handler(void)
{
	got_signal = 1;
}

static void do_child(unsigned int i)
{
	struct sigaction child_act;

	if (i == 0)
		child_act.sa_handler = SIG_IGN;
	else
		child_act.sa_handler = child_handler;

	child_act.sa_flags = SA_RESTART;
	sigemptyset(&child_act.sa_mask);

	SAFE_SIGACTION(SIGUSR2, &child_act, NULL);

	if ((ptrace(PTRACE_TRACEME, 0, 0, 0)) == -1) {
		tst_res(TWARN, "ptrace() failed in child");
		exit(1);
	}
	SAFE_KILL(getpid(), SIGUSR2);
	exit(1);
}

static void run(unsigned int i)
{
	pid_t child_pid;
	int status;
	struct sigaction parent_act;

	got_signal = 0;

	tst_res(TINFO, "%i", i);

	if (i == 1) {
		parent_act.sa_handler = parent_handler;
		parent_act.sa_flags = SA_RESTART;
		sigemptyset(&parent_act.sa_mask);

		SAFE_SIGACTION(SIGUSR2, &parent_act, NULL);
	}

	child_pid = SAFE_FORK();

	if (child_pid != 0) {

		SAFE_WAITPID(child_pid, &status, 0);

		if (((WIFEXITED(status))
			&& (WEXITSTATUS(status)))
				|| (got_signal == 1)) {
			tst_res(TFAIL, "Test Failed");
		} else {
			if ((ptrace(PTRACE_KILL, child_pid, 0, 0)) == -1) {
				tst_res(TFAIL | TERRNO,
					"Parent was not able to kill child");
			}
		}

		SAFE_WAITPID(child_pid, &status, 0);

		if (WTERMSIG(status) == 9)
			tst_res(TPASS, "Child %s", tst_strstatus(status));
		else
			tst_res(TFAIL, "Child %s", tst_strstatus(status));

	} else
		do_child(i);
}

static struct tst_test test = {
	.test = run,
	.tcnt = 2,
	.forks_child = 1,
};
