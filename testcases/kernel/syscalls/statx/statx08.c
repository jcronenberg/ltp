/*
 * Copyright (c) 2017 Instruction Ignorer <"can't"@be.bothered.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Test statx
 *
 * All tests should start with a description of _what_ we are testing.
 * Non-trivial explanations of _how_ the code works should also go here.
 * Include relevant links, Git commit hashes and CVE numbers.
 * Inline comments should be avoided.
 */

#include "tst_test.h"

static void run(void)
{
	tst_res(TPASS, "Doing hardly anything is easy");
}

static struct tst_test test = {
	.test_all = run,
	.min_kver = "4.11",
};
