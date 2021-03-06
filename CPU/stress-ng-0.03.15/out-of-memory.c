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
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "stress-ng.h"

#if defined (__linux__)
/*
 *  set_oom_adjustment()
 *	attempt to stop oom killer
 *	if we have root privileges then try and make process
 *	unkillable by oom killer
 */
void set_oom_adjustment(const char *name, bool killable)
{
	char path[PATH_MAX];
	int fd;
	bool high_priv;

	high_priv = (getuid() == 0) && (geteuid() == 0);

	/*
	 *  Try modern oom interface
	 */
	snprintf(path, sizeof(path), "/proc/%d/oom_score_adj", getpid());
	if ((fd = open(path, O_WRONLY)) >= 0) {
		char *str;
		ssize_t n;

		if (killable)
			str = "1000";
		else
			str = high_priv ? "-1000" : "0";

		n = write(fd, str, strlen(str));
		(void)close(fd);

		if (n < 0)
			pr_failed_dbg(name, "can't set oom_score_adj");
		else
			return;
	}
	/*
	 *  Fall back to old oom interface
	 */
	snprintf(path, sizeof(path), "/proc/%d/oom_adj", getpid());
	if ((fd = open(path, O_WRONLY)) >= 0) {
		char *str;
		ssize_t n;

		if (killable)
			str = high_priv ? "-17" : "-16";
		else
			str = "15";

		n = write(fd, str, strlen(str));
		(void)close(fd);

		if (n < 0)
			pr_failed_dbg(name, "can't set oom_adj");
	}
	return;
}
#else
void set_oom_adjustment(const char *name, bool killable)
{
	(void)name;
	(void)killable;
}
#endif
