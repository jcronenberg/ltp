// SPDX-License-Identifier: GPL-2.0-or-later
/**********************************************************
 *
 *    TEST IDENTIFIER	: ptrace01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: functionality test for ptrace(2)
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This test case tests the functionality of ptrace() for
 *	PTRACE_TRACEME & PTRACE_KILL requests.
 *	Here, we fork a child & the child does ptrace(PTRACE_TRACEME, ...).
 *	Then a signal is delivered to the child & verified that parent
 *	is notified via wait(). then parent does ptrace(PTRACE_KILL, ..)
 *	to kill the child. Again parent wait() for child to finish.
 *	If child finished abnormally, test passes.
 *		We test two cases
 * 			1) By telling child to ignore SIGUSR2 signal
 * 			2) By installing a signal handler for child for SIGUSR2
 * 		In both cases, child should stop & notify parent on reception
 * 		of SIGUSR2
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 setup signal handler for SIGUSR2 signal
 *	 fork a child
 *
 *	 CHILD:
 *		setup signal handler for SIGUSR2 signal
 *		call ptrace() with PTRACE_TRACEME request
 *		send SIGUSR2 signal to self
 *	 PARENT:
 *		wait() for child.
 *		if parent is notified when child gets a signal through wait(),
 *		then
 *			do  ptrace(PTRACE_KILL, ..) on child
 * 			wait() for child to finish,
 * 			if child exited abnormaly,
 *				TEST passed
 * 			else
 * 				TEST failed
 *		else
 *			TEST failed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  ptrace01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *			where,  -c n : Run n copies concurrently.
 *				-e   : Turn on errno logging.
 *				-h   : Show help screen
 *				-f   : Turn off functional testing
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 ****************************************************************/

#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include <config.h>
#include "ptrace.h"

#include "tst_test.h"

static int got_signal;

void child_handler(void)
{
	if ((kill(getppid(), SIGUSR2)) == -1) {
		tst_res(TWARN, "kill() failed in child_handler()");
		exit(1);
	}
}

void parent_handler(void)
{
	got_signal = 1;
}

void do_child(unsigned int i)
{
	struct sigaction child_act;

	if (i == 0)
		child_act.sa_handler = SIG_IGN;
	else
		child_act.sa_handler = child_handler;

	child_act.sa_flags = SA_RESTART;
	sigemptyset(&child_act.sa_mask);

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
		exit(1);
	}
	exit(1);
}

static void run(unsigned int i)
{

	pid_t child_pid;
	int status;
	struct sigaction parent_act;

#ifdef UCLINUX
	maybe_run_child(&do_child, "d", &i);
#endif

	got_signal = 0;

	if (i == 1) {
		parent_act.sa_handler = parent_handler;
		parent_act.sa_flags = SA_RESTART;
		sigemptyset(&parent_act.sa_mask);

		if ((sigaction(SIGUSR2, &parent_act, NULL))
		    == -1) {
			tst_res(TWARN, "sigaction() failed in parent");
		}
	}

	child_pid = SAFE_FORK();

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
			}
		}

		SAFE_WAITPID(child_pid, &status, 0);

		if (WIFEXITED(status))
			tst_res(TFAIL, "Test failed");
		else
			tst_res(TPASS, "Test Passed");

	} else
		do_child(i);
}

static struct tst_test test = {
	.test = run,
	.tcnt = 2,
	.forks_child = 1,
};
