/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * This code is a complete clean re-write of the stress tool by
 * Colin Ian King <colin.king@canonical.com> and attempts to be
 * backwardly compatible with the stress tool by Amos Waterland
 * <apw@rossby.metr.ou.edu> but has more stress tests and more
 * functionality.
 *
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>

#include "stress-ng.h"

/*
 *  stress_rename()
 *	stress system by renames
 */
int stress_rename(
	uint64_t *const counter,
	const uint32_t instance,
	const uint64_t max_ops,
	const char *name)
{
	char name1[PATH_MAX], name2[PATH_MAX];
	char *oldname = name1, *newname = name2, *tmpname;
	FILE *fp;
	uint64_t i = 0;
	const pid_t pid = getpid();

	if (stress_temp_dir_mk(name, pid, instance) < 0)
		return EXIT_FAILURE;
restart:
	(void)stress_temp_filename(oldname, PATH_MAX,
		name, pid, instance, i++);

	if ((fp = fopen(oldname, "w+")) == NULL) {
		pr_err(stderr, "%s: fopen failed: errno=%d: (%s)\n",
			name, errno, strerror(errno));
		(void)stress_temp_dir_rm(name, pid, instance);
		return EXIT_FAILURE;
	}
	(void)fclose(fp);

	do {
		(void)stress_temp_filename(newname, PATH_MAX,
			name, pid, instance, i++);
		if (rename(oldname, newname) < 0) {
			(void)unlink(oldname);
			(void)unlink(newname);
			goto restart;
		}

		tmpname = oldname;
		oldname = newname;
		newname = tmpname;

		(*counter)++;
	} while (opt_do_run && (!max_ops || *counter < max_ops));

	(void)unlink(oldname);
	(void)unlink(newname);
	(void)stress_temp_dir_rm(name, pid, instance);

	return EXIT_SUCCESS;
}
