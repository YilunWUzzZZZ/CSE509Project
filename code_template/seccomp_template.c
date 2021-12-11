#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

int seccomp(unsigned int operation, unsigned int flags,void *args) {
    return syscall(SYS_seccomp, operation, flags, args);
}

#define X32_SYSCALL_BIT 0x40000000U
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static int
install_filter()
{
    unsigned int upper_nr_limit = 0xffffffff;

    /* Assume that AUDIT_ARCH_X86_64 means the normal x86-64 ABI
      (in the x32 ABI, all system calls have bit 30 set in the
      'nr' field, meaning the numbers are >= X32_SYSCALL_BIT). */
    unsigned int t_arch = AUDIT_ARCH_X86_64;
    upper_nr_limit = X32_SYSCALL_BIT - 1;

    struct sock_filter filter[] = {
        /* insert */
    };

    struct sock_fprog prog = {
        .len = ARRAY_SIZE(filter),
        .filter = filter,
    };

    if (seccomp(SECCOMP_SET_MODE_FILTER, 0, &prog)) {
        perror("seccomp");
        return 1;
    }

    return 0;
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: "
                "%s <prog> [<args>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("prctl");
        exit(EXIT_FAILURE);
    }

    if (install_filter())
        exit(EXIT_FAILURE);

    execv(argv[1], &argv[1]);
    perror("execv");
    exit(EXIT_FAILURE);
}
