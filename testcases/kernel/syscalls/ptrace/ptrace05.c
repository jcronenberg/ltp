//SPDX-License-Identifier: GPL-2.0-or-later
/*
 ******************************************************************************
 *
 *   ptrace05 - an app which ptraces itself as per arbitrarily specified signals,
 *   over a user specified range.
 *
 *   Copyright (C) 2009, Ngie Cooper
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ******************************************************************************
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <libgen.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <config.h>
#include "ptrace.h"

#include "tst_test.h"
#include "lapi/signal.h"

void run(unsigned int i)
{

	int end_signum = -1;
	int signum;
	int start_signum = -1;
	int status;

	pid_t child;

	if (start_signum == -1) {
		start_signum = 0;
	}
	if (end_signum == -1) {
		end_signum = SIGRTMAX;
	}

	for (signum = start_signum; signum <= end_signum; signum++) {

		if (signum >= __SIGRTMIN && signum < SIGRTMIN)
			continue;

		child = SAFE_FORK();
		if (child == 0) {
			if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) != -1) {
				tst_res(TINFO, "[child] Sending kill(.., %d)",
					 signum);
				if (kill(getpid(), signum) < 0) {
					tst_res(TINFO | TERRNO,
						 "[child] kill(.., %d) failed.",
						 signum);
				}
			} else {

				/*
				 * This won't increment the TST_COUNT var.
				 * properly, but it'll show up as a failure
				 * nonetheless.
				 */
				tst_res(TFAIL | TERRNO,
					 "Failed to ptrace(PTRACE_TRACEME, ...) "
					 "properly");

			}
			/* Shouldn't get here if signum == 0. */
			exit((signum == 0 ? 0 : 2));
			break;
		} else {
			waitpid(child, &status, 0);

			switch (signum) {
			case 0:
				if (WIFEXITED(status)
				    && WEXITSTATUS(status) == 0) {
					tst_res(TPASS,
						 "kill(.., 0) exited "
						 "with 0, as expected.");
				} else {
					tst_res(TFAIL,
						 "kill(.., 0) didn't exit "
						 "with 0.");
				}
				break;
			case SIGKILL:
				if (WIFSIGNALED(status)) {
					/* SIGKILL must be uncatchable. */
					if (WTERMSIG(status) == SIGKILL) {
						tst_res(TPASS,
							 "Killed with SIGKILL, "
							 "as expected.");
					} else {
						tst_res(TPASS,
							 "Didn't die with "
							 "SIGKILL (?!) ");
					}
				} else if (WIFEXITED(status)) {
					tst_res(TFAIL,
						 "Exited unexpectedly instead "
						 "of dying with SIGKILL.");
				} else if (WIFSTOPPED(status)) {
					tst_res(TFAIL,
						 "Stopped instead of dying "
						 "with SIGKILL.");
				}
				break;
				/* All other processes should be stopped. */
			default:
				if (WIFSTOPPED(status)) {
					tst_res(TPASS, "Stopped as expected");
				} else {
					tst_res(TFAIL, "Didn't stop as "
						 "expected.");
					if (kill(child, 0)) {
						tst_res(TINFO,
							 "Is still alive!?");
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

				break;

			}
		}
		/* Make sure the child dies a quick and painless death ... */
		kill(child, 9);

	}

}

static struct tst_test test = {
	.test = run,
	.tcnt = 1,
	.forks_child = 1,
};
