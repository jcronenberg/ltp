// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2009, Ngie Cooper
 *   Copyright (c) 2019 SUSE LLC
 *
 *   Ported to new library: Jorik Cronenberg <jcronenberg@suse.de>
 *
 *   ptrace05 - an app which ptraces itself as per arbitrarily specified
 *   signals, over a user specified range.
 */

#include <stdlib.h>
#include <string.h>
#include <config.h>
#include "ptrace.h"
#include "tst_test.h"

static void run(void)
{

	int signum, status;
	int start_signum, end_signum;
	pid_t child_pid;

	start_signum = 0;
	end_signum = SIGRTMAX;

	for (signum = start_signum; signum <= end_signum; signum++) {

		if (signum >= __SIGRTMIN && signum < SIGRTMIN)
			continue;

		child_pid = SAFE_FORK();
		if (child_pid == 0) {
			if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) != -1) {
				tst_res(TINFO, "[child] Sending kill(.., %d)",
					signum);
				if (kill(getpid(), signum) < 0) {
					tst_res(TINFO | TERRNO,
						"[child] kill(.., %d) "
						"failed", signum);
				}
			} else {
				tst_res(TFAIL | TERRNO,
					"Failed to ptrace(PTRACE_TRACEME, ...) "
					"properly");
			}
			exit((signum == 0 ? 0 : 2));
		} else {
			SAFE_WAITPID(child_pid, &status, 0);

			switch (signum) {
			case 0:
				if (WIFEXITED(status)
				    && WEXITSTATUS(status) == 0) {
					tst_res(TPASS,
						 "kill(.., 0) exited "
						 "with 0, as expected");
				} else {
					tst_res(TFAIL,
						 "kill(.., 0) didn't exit "
						 "with 0");
				}
				break;
			case SIGKILL:
				if (WIFSIGNALED(status)) {

					if (WTERMSIG(status) == SIGKILL) {
						tst_res(TPASS,
							"Terminated with "
							"SIGKILL, as expected");
					} else {
						tst_res(TPASS,
							"Child didn't terminate"
							" with SIGKILL");
					}
				} else if (WIFEXITED(status)) {
					tst_res(TFAIL,
						"Exited unexpectedly instead "
						"of terminating with SIGKILL");
				} else if (WIFSTOPPED(status)) {
					tst_res(TFAIL,
						 "Stopped instead of exiting "
						 "with SIGKILL");
				}
				break;
			default:
				if (WIFSTOPPED(status)) {
					tst_res(TPASS, "Stopped as expected");
				} else {
					tst_res(TFAIL, "Didn't stop as "
						 "expected");
					if (kill(child_pid, 0)) {
						tst_res(TINFO,
							 "Process %i is still "
							 "alive", child_pid);
					} else if (WIFEXITED(status)) {
						tst_res(TINFO,
							 "Exited normally");
					} else if (WIFSIGNALED(status)) {
						tst_res(TINFO,
							 "Was signaled with "
							 "signum=%d",
							 WTERMSIG(status));
					}

				}

			SAFE_KILL(child_pid, 9);
			SAFE_WAITPID(child_pid, &status, 0);
			}

		}

	}

}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};
