<<<<<<< HEAD
// SPDX-License-Identifier: GPL-2.0
/*
=======
// SPDX-License-Identifier: GPL-2.0-or-later
/* 
>>>>>>> 90210e0180a4947e560dd525185118db8d8aa97c
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
<<<<<<< HEAD
 *			1) By telling child to ignore SIGUSR2 signal
 *			2) By installing a signal handler for child for SIGUSR2
 *		In both cases, child should stop & notify parent on reception
 *		of SIGUSR2
=======
 * 			1) By telling child to ignore SIGUSR2 signal
 * 			2) By installing a signal handler for child for SIGUSR2
 * 		In both cases, child should stop & notify parent on reception
 * 		of SIGUSR2
>>>>>>> 90210e0180a4947e560dd525185118db8d8aa97c
 *
 */

#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <config.h>
#include "ptrace.h"
#include "tst_test.h"

<<<<<<< HEAD
static volatile int got_signal;

static void child_handler(void)
{
	SAFE_KILL(getppid(), SIGUSR2);
	tst_res(TINFO, "child_handler ran"); //delete
}

static void parent_handler(void)
=======
static int got_signal;

void child_handler(void)
{
	if ((kill(getppid(), SIGUSR2)) == -1) {
		tst_res(TWARN, "kill() failed in child_handler()");
		exit(1);
	}
}

void parent_handler(void)
>>>>>>> 90210e0180a4947e560dd525185118db8d8aa97c
{
	got_signal = 1;
}

<<<<<<< HEAD
static void do_child(unsigned int i)
=======
void do_child(unsigned int i)
>>>>>>> 90210e0180a4947e560dd525185118db8d8aa97c
{
	struct sigaction child_act;

	if (i == 0)
		child_act.sa_handler = SIG_IGN;
	else
		child_act.sa_handler = child_handler;

	child_act.sa_flags = SA_RESTART;
	sigemptyset(&child_act.sa_mask);

<<<<<<< HEAD
	SAFE_SIGACTION(SIGUSR2, &child_act, NULL);

	if ((ptrace(PTRACE_TRACEME, 0, 0, 0)) == -1) {
		tst_res(TWARN, "ptrace() failed in child");
=======
	if ((sigaction(SIGUSR2, &child_act, NULL)) == -1) {
		tst_res(TWARN, "sigaction() failed in child");
		exit(1);
	}

	if ((ptrace(PTRACE_TRACEME, 0, 0, 0)) == -1) {
		tst_res(TWARN, "ptrace() failed in child");
		exit(1);
	}
	if ((kill(getpid(), SIGUSR2)) == -1) {
		tst_res(TWARN, "kill() failed in child");
>>>>>>> 90210e0180a4947e560dd525185118db8d8aa97c
		exit(1);
	}
	SAFE_KILL(getpid(), SIGUSR2);
	tst_res(TINFO, "do_child ran"); //delete
	exit(1);
}

static void run(unsigned int i)
{
	pid_t child_pid;
	int status;
	struct sigaction parent_act;

<<<<<<< HEAD
	int tmp;

	got_signal = 0;

	tst_res(TINFO, "%i", i);
=======
	pid_t child_pid;
	int status;
	struct sigaction parent_act;

#ifdef UCLINUX
	maybe_run_child(&do_child, "d", &i);
#endif

	got_signal = 0;
>>>>>>> 90210e0180a4947e560dd525185118db8d8aa97c

	if (i == 1) {
		parent_act.sa_handler = parent_handler;
		parent_act.sa_flags = SA_RESTART;
		sigemptyset(&parent_act.sa_mask);

<<<<<<< HEAD
		SAFE_SIGACTION(SIGUSR2, &parent_act, NULL);
=======
		if ((sigaction(SIGUSR2, &parent_act, NULL))
		    == -1) {
			tst_res(TWARN, "sigaction() failed in parent");
		}
>>>>>>> 90210e0180a4947e560dd525185118db8d8aa97c
	}

	child_pid = SAFE_FORK();

<<<<<<< HEAD
	tst_res(TINFO, "child_pid: %i", child_pid); //delete

	if (child_pid != 0) {

		SAFE_WAITPID(child_pid, &status, 0);

		if (((WIFEXITED(status))
			&& (WEXITSTATUS(status)))
			 || (got_signal == 1)) {
			tst_res(TFAIL, "Test Failed");
		} else {
			tmp = ptrace(PTRACE_KILL, child_pid, 0, 0);
			tst_res(TINFO, "Signal: %i", tmp); //delete note for tomorrow investigate tst_strstatus()
			if (/*(ptrace(PTRACE_KILL, child_pid, 0, 0))*/tmp == -1) {
				tst_res(TFAIL,
					"Test Failed: Parent was not able to kill child");
=======
	if (child_pid != 0) {

		SAFE_WAITPID(child_pid, &status, 0);

		if (((WIFEXITED(status)) &&
		     (WEXITSTATUS(status))) ||
		    (got_signal == 1)) {
			tst_res(TFAIL, "Test Failed");
		} else {
			if ((ptrace(PTRACE_KILL, child_pid,
				    0, 0)) == -1) {
				tst_res(TFAIL,
						"Test Failed: Parent was not able to kill child");
>>>>>>> 90210e0180a4947e560dd525185118db8d8aa97c
			}
		}

		SAFE_WAITPID(child_pid, &status, 0);

		if (WIFEXITED(status))
			tst_res(TFAIL, "Test failed");
		else
<<<<<<< HEAD
			tst_res(TPASS, "Test passed");
=======
			tst_res(TPASS, "Test Passed");
>>>>>>> 90210e0180a4947e560dd525185118db8d8aa97c

	} else
		do_child(i);
}

static struct tst_test test = {
	.test = run,
	.tcnt = 2,
	.forks_child = 1,
};
