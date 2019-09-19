// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Jorik Cronenberg <jcronenberg@suse.com>
 *
 * Trivial Extended Berkeley Packet Filter (eBPF) test.
 *
 * Sanity check creating and updating maps.
 */

#include <stdint.h>
//#include <stdio.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

struct clone_args {
        uint64_t flags;
        uint64_t *pidfd;
        uint64_t *child_tid;
        uint64_t *parent_tid;
        uint64_t exit_signal;
        uint64_t stack;
        uint64_t stack_size;
        uint64_t tls;
};

static int size;

static long sys_clone3(struct clone_args *uargs, size_t size)
{
	return tst_syscall(__NR_clone3, uargs, size);
}


static void run(void)
{
	struct clone_args uargs = {0};
	size = sizeof(uargs);
	TEST(sys_clone3(&uargs, size));

	//printf("\n%li:%i\n", TST_RET,size);
	/*printf("flags:%li\n", uargs.flags);
	printf("exit_signal:%li\n", uargs.exit_signal);
	printf("stack:%li\n", uargs.stack);
	printf("stack_size:%li\n", uargs.stack_size);
	printf("tls:%li\n", uargs.tls);*/


	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "clone3 set errno to some unexpected value");
	else
		tst_res(TPASS, "clone3 worked as expected");


	/*TEST(sys_clone3(&uargs, 0));
	if (TST_RET == -1)
		tst_res(TPASS, "clone3 failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "clone3 worked which wasn't intended");*/
}

static struct tst_test test = {
	.test_all = run,
	.min_kver = "5.3",
	.needs_tmpdir = 1,
};
