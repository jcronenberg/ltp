/*
 * Test statx
 *
 * Check if statx exists and what error code it returns when we give it dodgy
 * data. Then stat a file and check it returns success.
 */

#include <stdio.h>
#include <stdint.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/fcntl.h"

static int fd, lfd;

#define LNAME "file_to_stat_link"
#define FNAME "file_to_stat"
#define STATX_BASIC_STATS 0x000007ffU

struct statx_timestamp {
	int64_t	       tv_sec;
	uint32_t       tv_nsec;
	int32_t	       __reserved;
};

struct statx {
	uint32_t	stx_mask;
	uint32_t	stx_blksize;
	uint64_t	stx_attributes;
	uint32_t	stx_nlink;
	uint32_t	stx_uid;
	uint32_t	stx_gid;
	uint16_t	stx_mode;
	uint16_t	__spare0[1];
	uint64_t	stx_ino;
	uint64_t	stx_size;
	uint64_t	stx_blocks;
	uint64_t	stx_attributes_mask;
	struct statx_timestamp	stx_atime;
	struct statx_timestamp	stx_btime;
	struct statx_timestamp	stx_ctime;
	struct statx_timestamp	stx_mtime;
	uint32_t	stx_rdev_major;
	uint32_t	stx_rdev_minor;
	uint32_t	stx_dev_major;
	uint32_t	stx_dev_minor;
	uint64_t	__spare2[14];
};

static int sys_statx(int dirfd, const char *pathname, int flags,
		     unsigned int mask, struct statx *statxbuf)
{
	return tst_syscall(__NR_statx, dirfd, pathname, flags, mask, statxbuf);
}

static void setup(void)
{
        fd = SAFE_OPEN(FNAME, O_CREAT, 0777);
        SAFE_LINK(FNAME, LNAME);
        lfd = SAFE_OPEN(LNAME, 0); 
}

static void cleanup(void)
{
        if (lfd != 0)
                SAFE_CLOSE(lfd);

        if (fd != 0)
                SAFE_CLOSE(fd);
}


static void run(void)
{
	struct statx statxbuf = { 0 };

	TEST(sys_statx(0, NULL, 0, 0, &statxbuf));
	if (TST_RET == 0)
		tst_res(TFAIL, "statx thinks it can stat NULL");
	else if (TST_ERR == EFAULT)
		tst_res(TPASS, "statx set errno to EFAULT as expected");
	else
		tst_res(TFAIL | TERRNO, "statx set errno to some unexpected value");

	TEST(sys_statx(AT_FDCWD, FNAME, 0, STATX_BASIC_STATS, &statxbuf));
	if (TST_RET == 0)
		tst_res(TPASS, "It returned zero so it must have worked!");
	else
		tst_res(TFAIL | TERRNO, "statx can not stat a basic file");

	TEST(sys_statx(AT_FDCWD, LNAME, 0, STATX_BASIC_STATS, &statxbuf));
	if (TST_RET == 0)
		tst_res(TPASS, "It returned zero so it must have worked!");
	else
		tst_res(TFAIL | TERRNO, "statx can not stat a basic file");

	printf("statx.stx_nlink = %d", statxbuf.stx_nlink);

}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	//.tcnt = 2,
	.min_kver = "4.11",
	.needs_tmpdir = 1
};
