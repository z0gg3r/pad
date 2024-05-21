/* SPDX-License-Identifier: Zlib */
/*
 * seccomp filtering
 *
 * stolen from zathura
 */

#include "seccomp.h"

#include <seccomp.h> /* libseccomp */
#include <sys/prctl.h> /* prctl */
#include <stdlib.h>
#include <errno.h>
#include <linux/sched.h> /* for clone filter */
#include <fcntl.h>

#define ADD_RULE(str_action, action, call, ...)                               \
	do {                                                                  \
		const int err = seccomp_rule_add(ctx, action, SCMP_SYS(call), \
						 __VA_ARGS__);                \
		if (err < 0) {                                                \
			goto out;                                             \
		}                                                             \
	} while (0)

#define ALLOW_RULE(call) ADD_RULE("allow", SCMP_ACT_ALLOW, call, 0)
#define ALLOW_ONLY_RULE(call, ...) ADD_RULE("allow", SCMP_ACT_ALLOW, call, 1, __VA_ARGS__)
#define ERRNO_RULE(call) ADD_RULE("errno", SCMP_ACT_ERRNO(ENOSYS), call, 0)

int seccomp_enable_strict_filter(void)
{
	/* prevent child processes from getting more priv e.g. via setuid, capabilities, ... */
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
		return -1;
	}

	/* prevent escape via ptrace */
	if (prctl(PR_SET_DUMPABLE, 0, 0, 0, 0)) {
		return -1;
	}

	/* initialize the filter */
	scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL_PROCESS);
	if (ctx == NULL) {
		return -1;
	}

	/* Generic rules */
	ALLOW_RULE(close);
	ALLOW_RULE(exit);
	ALLOW_RULE(exit_group);
	ALLOW_RULE(read);
	ALLOW_RULE(ioctl); /* ioctl -- TODO: restrict */
	ALLOW_RULE(brk);
	ALLOW_RULE(mmap);
	ALLOW_RULE(munmap);
	ALLOW_RULE(mprotect);
	ALLOW_RULE(pread64);
	ALLOW_RULE(fstat);

	/* Specific rules */
	ALLOW_ONLY_RULE(open, SCMP_CMP(2, SCMP_CMP_MASKED_EQ, O_RDONLY, 0));
	ALLOW_ONLY_RULE(openat, SCMP_CMP(2, SCMP_CMP_MASKED_EQ, O_RDONLY, 0));
	ALLOW_ONLY_RULE(write, SCMP_CMP(0, SCMP_CMP_EQ, 1));
	ALLOW_ONLY_RULE(write, SCMP_CMP(0, SCMP_CMP_EQ, 2));

	/* applying filter... */
	if (seccomp_load(ctx) >= 0) {
		/* free ctx after the filter has been loaded into the kernel */
		seccomp_release(ctx);
		return 0;
	}

out:
	/* something went wrong */
	seccomp_release(ctx);
	return -1;
}