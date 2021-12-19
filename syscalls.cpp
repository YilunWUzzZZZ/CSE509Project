#include "syscalls.h"
#include <unordered_map>
#include <set>

string syscall_regs[] = {"rdi", "rsi", "rdx", "r10", "r8", "r9"};
set<string> char_star_as_buffer_type = {"write", "read"};

SyscallInfo syscall_infos[] = {
{ .nr = 0, .name = "read", .num_args = 3,
.arg_types={ 
"unsigned int",
"char *",
"size_t",
}
},
{ .nr = 1, .name = "write", .num_args = 3,
.arg_types={ 
"unsigned int",
"const char *",
"size_t",
}
},
{ .nr = 2, .name = "open", .num_args = 3,
.arg_types={ 
"const char *",
"int",
"int",
}
},
{ .nr = 3, .name = "close", .num_args = 1,
.arg_types={ 
"unsigned int",
}
},
{ .nr = 4, .name = "stat", .num_args = 2,
.arg_types={ 
"const char *",
"struct stat *",
}
},
{ .nr = 5, .name = "fstat", .num_args = 2,
.arg_types={ 
"unsigned int",
"struct stat *",
}
},
{ .nr = 6, .name = "lstat", .num_args = 2,
.arg_types={ 
"fconst char *",
"struct stat *",
}
},
{ .nr = 7, .name = "poll", .num_args = 3,
.arg_types={ 
"struct poll_fd *",
"unsigned int",
"long",
}
},
{ .nr = 8, .name = "lseek", .num_args = 3,
.arg_types={ 
"unsigned int",
"off_t",
"unsigned int",
}
},
{ .nr = 9, .name = "mmap", .num_args = 6,
.arg_types={ 
"unsigned long",
"unsigned long",
"unsigned long",
"unsigned long",
"unsigned long",
"unsigned long",
}
},
{ .nr = 10, .name = "mprotect", .num_args = 3,
.arg_types={ 
"unsigned long",
"size_t",
"unsigned long",
}
},
{ .nr = 11, .name = "munmap", .num_args = 2,
.arg_types={ 
"unsigned long",
"size_t",
}
},
{ .nr = 12, .name = "brk", .num_args = 1,
.arg_types={ 
"unsigned long",
}
},
{ .nr = 13, .name = "rt_sigaction", .num_args = 4,
.arg_types={ 
"int",
"const struct sigaction *",
"struct sigaction *",
"size_t",
}
},
{ .nr = 14, .name = "rt_sigprocmask", .num_args = 4,
.arg_types={ 
"int",
"sigset_t *",
"sigset_t *",
"size_t",
}
},
{ .nr = 15, .name = "rt_sigreturn", .num_args = 1,
.arg_types={ 
"unsigned long",
}
},
{ .nr = 16, .name = "ioctl", .num_args = 3,
.arg_types={ 
"unsigned int",
"unsigned int",
"unsigned long",
}
},
{ .nr = 17, .name = "pread64", .num_args = 4,
.arg_types={ 
"unsigned long",
"char *",
"size_t",
"loff_t",
}
},
{ .nr = 18, .name = "pwrite64", .num_args = 4,
.arg_types={ 
"unsigned int",
"const char *",
"size_t",
"loff_t",
}
},
{ .nr = 19, .name = "readv", .num_args = 3,
.arg_types={ 
"unsigned long",
"const struct iovec *",
"unsigned long",
}
},
{ .nr = 20, .name = "writev", .num_args = 3,
.arg_types={ 
"unsigned long",
"const struct iovec *",
"unsigned long",
}
},
{ .nr = 21, .name = "access", .num_args = 2,
.arg_types={ 
"const char *",
"int",
}
},
{ .nr = 22, .name = "pipe", .num_args = 1,
.arg_types={ 
"int *",
}
},
{ .nr = 23, .name = "select", .num_args = 5,
.arg_types={ 
"int",
"fd_set *",
"fd_set *",
"",
"struct timeval *",
}
},
{ .nr = 24, .name = "sched_yield", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 25, .name = "mremap", .num_args = 5,
.arg_types={ 
"unsigned long",
"unsigned long",
"unsigned long",
"unsigned long",
"unsigned long",
}
},
{ .nr = 26, .name = "msync", .num_args = 3,
.arg_types={ 
"unsigned long",
"size_t",
"int",
}
},
{ .nr = 27, .name = "mincore", .num_args = 3,
.arg_types={ 
"unsigned long",
"size_t",
"unsigned char *",
}
},
{ .nr = 28, .name = "madvise", .num_args = 3,
.arg_types={ 
"unsigned long",
"size_t",
"int",
}
},
{ .nr = 29, .name = "shmget", .num_args = 3,
.arg_types={ 
"key_t",
"size_t",
"int",
}
},
{ .nr = 30, .name = "shmat", .num_args = 3,
.arg_types={ 
"int",
"char *",
"int",
}
},
{ .nr = 31, .name = "shmctl", .num_args = 3,
.arg_types={ 
"int",
"int",
"struct shmid_ds *",
}
},
{ .nr = 32, .name = "dup", .num_args = 1,
.arg_types={ 
"unsigned int",
}
},
{ .nr = 33, .name = "dup2", .num_args = 2,
.arg_types={ 
"unsigned int",
"unsigned int",
}
},
{ .nr = 34, .name = "pause", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 35, .name = "nanosleep", .num_args = 2,
.arg_types={ 
"struct timespec *",
"struct timespec *",
}
},
{ .nr = 36, .name = "getitimer", .num_args = 2,
.arg_types={ 
"int",
"struct itimerval *",
}
},
{ .nr = 37, .name = "alarm", .num_args = 1,
.arg_types={ 
"unsigned int",
}
},
{ .nr = 38, .name = "setitimer", .num_args = 3,
.arg_types={ 
"int",
"struct itimerval *",
"struct itimerval *",
}
},
{ .nr = 39, .name = "getpid", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 40, .name = "sendfile", .num_args = 4,
.arg_types={ 
"int",
"int",
"off_t *",
"size_t",
}
},
{ .nr = 41, .name = "socket", .num_args = 3,
.arg_types={ 
"int",
"int",
"int",
}
},
{ .nr = 42, .name = "connect", .num_args = 3,
.arg_types={ 
"int",
"struct sockaddr *",
"int",
}
},
{ .nr = 43, .name = "accept", .num_args = 3,
.arg_types={ 
"int",
"struct sockaddr *",
"int *",
}
},
{ .nr = 44, .name = "sendto", .num_args = 6,
.arg_types={ 
"int",
"void *",
"size_t",
"unsigned",
"struct sockaddr *",
"int",
}
},
{ .nr = 45, .name = "recvfrom", .num_args = 6,
.arg_types={ 
"int",
"void *",
"size_t",
"unsigned",
"struct sockaddr *",
"int *",
}
},
{ .nr = 46, .name = "sendmsg", .num_args = 3,
.arg_types={ 
"int",
"struct msghdr *",
"unsigned",
}
},
{ .nr = 47, .name = "recvmsg", .num_args = 3,
.arg_types={ 
"int",
"struct msghdr *",
"unsigned int",
}
},
{ .nr = 48, .name = "shutdown", .num_args = 2,
.arg_types={ 
"int",
"int",
}
},
{ .nr = 49, .name = "bind", .num_args = 3,
.arg_types={ 
"int",
"struct sockaddr *",
"int",
}
},
{ .nr = 50, .name = "listen", .num_args = 2,
.arg_types={ 
"int",
"int",
}
},
{ .nr = 51, .name = "getsockname", .num_args = 3,
.arg_types={ 
"int",
"struct sockaddr *",
"int *",
}
},
{ .nr = 52, .name = "getpeername", .num_args = 3,
.arg_types={ 
"int",
"struct sockaddr *",
"int *",
}
},
{ .nr = 53, .name = "socketpair", .num_args = 4,
.arg_types={ 
"int",
"int",
"int",
"int *",
}
},
{ .nr = 54, .name = "setsockopt", .num_args = 5,
.arg_types={ 
"int",
"int",
"int",
"char *",
"int",
}
},
{ .nr = 55, .name = "getsockopt", .num_args = 5,
.arg_types={ 
"int",
"int",
"int",
"char *",
"int *",
}
},
{ .nr = 56, .name = "clone", .num_args = 5,
.arg_types={ 
"unsigned long",
"unsigned long",
"void *",
"void *",
"unsigned int",
}
},
{ .nr = 57, .name = "fork", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 58, .name = "vfork", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 59, .name = "execve", .num_args = 3,
.arg_types={ 
"const char *",
"const char *const",
"const char *const",
}
},
{ .nr = 60, .name = "exit", .num_args = 1,
.arg_types={ 
"int",
}
},
{ .nr = 61, .name = "wait4", .num_args = 4,
.arg_types={ 
"pid_t",
"int *",
"int",
"struct rusage *",
}
},
{ .nr = 62, .name = "kill", .num_args = 2,
.arg_types={ 
"pid_t",
"int",
}
},
{ .nr = 63, .name = "uname", .num_args = 1,
.arg_types={ 
"struct old_utsname *",
}
},
{ .nr = 64, .name = "semget", .num_args = 3,
.arg_types={ 
"key_t",
"int",
"int",
}
},
{ .nr = 65, .name = "semop", .num_args = 3,
.arg_types={ 
"int",
"struct sembuf *",
"unsigned",
}
},
{ .nr = 66, .name = "semctl", .num_args = 4,
.arg_types={ 
"int",
"int",
"int",
"union semun",
}
},
{ .nr = 67, .name = "shmdt", .num_args = 1,
.arg_types={ 
"char *",
}
},
{ .nr = 68, .name = "msgget", .num_args = 2,
.arg_types={ 
"key_t",
"int",
}
},
{ .nr = 69, .name = "msgsnd", .num_args = 4,
.arg_types={ 
"int",
"struct msgbuf *",
"size_t",
"int",
}
},
{ .nr = 70, .name = "msgrcv", .num_args = 5,
.arg_types={ 
"int",
"struct msgbuf *",
"size_t",
"long",
"int",
}
},
{ .nr = 71, .name = "msgctl", .num_args = 3,
.arg_types={ 
"int",
"int",
"struct msqid_ds *",
}
},
{ .nr = 72, .name = "fcntl", .num_args = 3,
.arg_types={ 
"unsigned int",
"unsigned int",
"unsigned long",
}
},
{ .nr = 73, .name = "flock", .num_args = 2,
.arg_types={ 
"unsigned int",
"unsigned int",
}
},
{ .nr = 74, .name = "fsync", .num_args = 1,
.arg_types={ 
"unsigned int",
}
},
{ .nr = 75, .name = "fdatasync", .num_args = 1,
.arg_types={ 
"unsigned int",
}
},
{ .nr = 76, .name = "truncate", .num_args = 2,
.arg_types={ 
"const char *",
"long",
}
},
{ .nr = 77, .name = "ftruncate", .num_args = 2,
.arg_types={ 
"unsigned int",
"unsigned long",
}
},
{ .nr = 78, .name = "getdents", .num_args = 3,
.arg_types={ 
"unsigned int",
"struct linux_dirent *",
"unsigned int",
}
},
{ .nr = 79, .name = "getcwd", .num_args = 2,
.arg_types={ 
"char *",
"unsigned long",
}
},
{ .nr = 80, .name = "chdir", .num_args = 1,
.arg_types={ 
"const char *",
}
},
{ .nr = 81, .name = "fchdir", .num_args = 1,
.arg_types={ 
"unsigned int",
}
},
{ .nr = 82, .name = "rename", .num_args = 2,
.arg_types={ 
"const char *",
"const char *",
}
},
{ .nr = 83, .name = "mkdir", .num_args = 2,
.arg_types={ 
"const char *",
"int",
}
},
{ .nr = 84, .name = "rmdir", .num_args = 1,
.arg_types={ 
"const char *",
}
},
{ .nr = 85, .name = "creat", .num_args = 2,
.arg_types={ 
"const char *",
"int",
}
},
{ .nr = 86, .name = "link", .num_args = 2,
.arg_types={ 
"const char *",
"const char *",
}
},
{ .nr = 87, .name = "unlink", .num_args = 1,
.arg_types={ 
"const char *",
}
},
{ .nr = 88, .name = "symlink", .num_args = 2,
.arg_types={ 
"const char *",
"const char *",
}
},
{ .nr = 89, .name = "readlink", .num_args = 3,
.arg_types={ 
"const char *",
"char *",
"int",
}
},
{ .nr = 90, .name = "chmod", .num_args = 2,
.arg_types={ 
"const char *",
"mode_t",
}
},
{ .nr = 91, .name = "fchmod", .num_args = 2,
.arg_types={ 
"unsigned int",
"mode_t",
}
},
{ .nr = 92, .name = "chown", .num_args = 3,
.arg_types={ 
"const char *",
"uid_t",
"gid_t",
}
},
{ .nr = 93, .name = "fchown", .num_args = 3,
.arg_types={ 
"unsigned int",
"uid_t",
"gid_t",
}
},
{ .nr = 94, .name = "lchown", .num_args = 3,
.arg_types={ 
"const char *",
"uid_t",
"gid_t",
}
},
{ .nr = 95, .name = "umask", .num_args = 1,
.arg_types={ 
"int",
}
},
{ .nr = 96, .name = "gettimeofday", .num_args = 2,
.arg_types={ 
"struct timeval *",
"struct timezone *",
}
},
{ .nr = 97, .name = "getrlimit", .num_args = 2,
.arg_types={ 
"unsigned int",
"struct rlimit *",
}
},
{ .nr = 98, .name = "getrusage", .num_args = 2,
.arg_types={ 
"int",
"struct rusage *",
}
},
{ .nr = 99, .name = "sysinfo", .num_args = 1,
.arg_types={ 
"struct sysinfo *",
}
},
{ .nr = 100, .name = "times", .num_args = 1,
.arg_types={ 
"struct tms *",
}
},
{ .nr = 101, .name = "ptrace", .num_args = 4,
.arg_types={ 
"long",
"long",
"unsigned long",
"unsigned long",
}
},
{ .nr = 102, .name = "getuid", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 103, .name = "syslog", .num_args = 3,
.arg_types={ 
"int",
"char *",
"int",
}
},
{ .nr = 104, .name = "getgid", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 105, .name = "setuid", .num_args = 1,
.arg_types={ 
"uid_t",
}
},
{ .nr = 106, .name = "setgid", .num_args = 1,
.arg_types={ 
"gid_t",
}
},
{ .nr = 107, .name = "geteuid", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 108, .name = "getegid", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 109, .name = "setpgid", .num_args = 2,
.arg_types={ 
"pid_t",
"pid_t",
}
},
{ .nr = 110, .name = "getppid", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 111, .name = "getpgrp", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 112, .name = "setsid", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 113, .name = "setreuid", .num_args = 2,
.arg_types={ 
"uid_t",
"uid_t",
}
},
{ .nr = 114, .name = "setregid", .num_args = 2,
.arg_types={ 
"gid_t",
"gid_t",
}
},
{ .nr = 115, .name = "getgroups", .num_args = 2,
.arg_types={ 
"int",
"gid_t *",
}
},
{ .nr = 116, .name = "setgroups", .num_args = 2,
.arg_types={ 
"int",
"gid_t *",
}
},
{ .nr = 117, .name = "setresuid", .num_args = 3,
.arg_types={ 
"uid_t *",
"uid_t *",
"uid_t *",
}
},
{ .nr = 118, .name = "getresuid", .num_args = 3,
.arg_types={ 
"uid_t *",
"uid_t *",
"uid_t *",
}
},
{ .nr = 119, .name = "setresgid", .num_args = 3,
.arg_types={ 
"gid_t",
"gid_t",
"gid_t",
}
},
{ .nr = 120, .name = "getresgid", .num_args = 3,
.arg_types={ 
"gid_t *",
"gid_t *",
"gid_t *",
}
},
{ .nr = 121, .name = "getpgid", .num_args = 1,
.arg_types={ 
"pid_t",
}
},
{ .nr = 122, .name = "setfsuid", .num_args = 1,
.arg_types={ 
"uid_t",
}
},
{ .nr = 123, .name = "setfsgid", .num_args = 1,
.arg_types={ 
"gid_t",
}
},
{ .nr = 124, .name = "getsid", .num_args = 1,
.arg_types={ 
"pid_t",
}
},
{ .nr = 125, .name = "capget", .num_args = 2,
.arg_types={ 
"cap_user_header_t",
"cap_user_data_t",
}
},
{ .nr = 126, .name = "capset", .num_args = 2,
.arg_types={ 
"cap_user_header_t",
"const cap_user_data_t",
}
},
{ .nr = 127, .name = "rt_sigpending", .num_args = 2,
.arg_types={ 
"sigset_t *",
"size_t",
}
},
{ .nr = 128, .name = "rt_sigtimedwait", .num_args = 4,
.arg_types={ 
"const sigset_t *",
"siginfo_t *",
"const struct timespec *",
"size_t",
}
},
{ .nr = 129, .name = "rt_sigqueueinfo", .num_args = 3,
.arg_types={ 
"pid_t",
"int",
"siginfo_t *",
}
},
{ .nr = 130, .name = "rt_sigsuspend", .num_args = 2,
.arg_types={ 
"sigset_t *",
"size_t",
}
},
{ .nr = 131, .name = "sigaltstack", .num_args = 2,
.arg_types={ 
"const stack_t *",
"stack_t *",
}
},
{ .nr = 132, .name = "utime", .num_args = 2,
.arg_types={ 
"char *",
"struct utimbuf *",
}
},
{ .nr = 133, .name = "mknod", .num_args = 3,
.arg_types={ 
"const char *",
"umode_t",
"unsigned",
}
},
{ .nr = 134, .name = "uselib", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 135, .name = "personality", .num_args = 1,
.arg_types={ 
"unsigned int",
}
},
{ .nr = 136, .name = "ustat", .num_args = 2,
.arg_types={ 
"unsigned",
"struct ustat *",
}
},
{ .nr = 137, .name = "statfs", .num_args = 2,
.arg_types={ 
"const char *",
"struct statfs *",
}
},
{ .nr = 138, .name = "fstatfs", .num_args = 2,
.arg_types={ 
"unsigned int",
"struct statfs *",
}
},
{ .nr = 139, .name = "sysfs", .num_args = 3,
.arg_types={ 
"int",
"unsigned long",
"unsigned long",
}
},
{ .nr = 140, .name = "getpriority", .num_args = 2,
.arg_types={ 
"int",
"int",
}
},
{ .nr = 141, .name = "setpriority", .num_args = 3,
.arg_types={ 
"int",
"int",
"int",
}
},
{ .nr = 142, .name = "sched_setparam", .num_args = 2,
.arg_types={ 
"pid_t",
"struct sched_param *",
}
},
{ .nr = 143, .name = "sched_getparam", .num_args = 2,
.arg_types={ 
"pid_t",
"struct sched_param *",
}
},
{ .nr = 144, .name = "sched_setscheduler", .num_args = 3,
.arg_types={ 
"pid_t",
"int",
"struct sched_param *",
}
},
{ .nr = 145, .name = "sched_getscheduler", .num_args = 1,
.arg_types={ 
"pid_t",
}
},
{ .nr = 146, .name = "sched_get_priority_max", .num_args = 1,
.arg_types={ 
"int",
}
},
{ .nr = 147, .name = "sched_get_priority_min", .num_args = 1,
.arg_types={ 
"int",
}
},
{ .nr = 148, .name = "sched_rr_get_interval", .num_args = 2,
.arg_types={ 
"pid_t",
"struct timespec *",
}
},
{ .nr = 149, .name = "mlock", .num_args = 2,
.arg_types={ 
"unsigned long",
"size_t",
}
},
{ .nr = 150, .name = "munlock", .num_args = 2,
.arg_types={ 
"unsigned long",
"size_t",
}
},
{ .nr = 151, .name = "mlockall", .num_args = 1,
.arg_types={ 
"int",
}
},
{ .nr = 152, .name = "munlockall", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 153, .name = "vhangup", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 154, .name = "modify_ldt", .num_args = 3,
.arg_types={ 
"int",
"void *",
"unsigned long",
}
},
{ .nr = 155, .name = "pivot_root", .num_args = 2,
.arg_types={ 
"const char *",
"const char *",
}
},
{ .nr = 156, .name = "_sysctl", .num_args = 1,
.arg_types={ 
"struct __sysctl_args *",
}
},
{ .nr = 157, .name = "prctl", .num_args = 4,
.arg_types={ 
"int",
"unsigned long",
"unsigned long",
"unsigned long",
}
},
{ .nr = 158, .name = "arch_prctl", .num_args = 3,
.arg_types={ 
"struct task_struct *",
"int",
"unsigned long *",
}
},
{ .nr = 159, .name = "adjtimex", .num_args = 1,
.arg_types={ 
"struct timex *",
}
},
{ .nr = 160, .name = "setrlimit", .num_args = 2,
.arg_types={ 
"unsigned int",
"struct rlimit *",
}
},
{ .nr = 161, .name = "chroot", .num_args = 1,
.arg_types={ 
"const char *",
}
},
{ .nr = 162, .name = "sync", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 163, .name = "acct", .num_args = 1,
.arg_types={ 
"const char *",
}
},
{ .nr = 164, .name = "settimeofday", .num_args = 2,
.arg_types={ 
"struct timeval *",
"struct timezone *",
}
},
{ .nr = 165, .name = "mount", .num_args = 5,
.arg_types={ 
"char *",
"char *",
"char *",
"unsigned long",
"void *",
}
},
{ .nr = 166, .name = "umount2", .num_args = 2,
.arg_types={ 
"const char *",
"int",
}
},
{ .nr = 167, .name = "swapon", .num_args = 2,
.arg_types={ 
"const char *",
"int",
}
},
{ .nr = 168, .name = "swapoff", .num_args = 1,
.arg_types={ 
"const char *",
}
},
{ .nr = 169, .name = "reboot", .num_args = 4,
.arg_types={ 
"int",
"int",
"unsigned int",
"void *",
}
},
{ .nr = 170, .name = "sethostname", .num_args = 2,
.arg_types={ 
"char *",
"int",
}
},
{ .nr = 171, .name = "setdomainname", .num_args = 2,
.arg_types={ 
"char *",
"int",
}
},
{ .nr = 172, .name = "iopl", .num_args = 2,
.arg_types={ 
"unsigned int",
"struct pt_regs *",
}
},
{ .nr = 173, .name = "ioperm", .num_args = 3,
.arg_types={ 
"unsigned long",
"unsigned long",
"int",
}
},
{ .nr = 174, .name = "create_module", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 175, .name = "init_module", .num_args = 3,
.arg_types={ 
"void *",
"unsigned long",
"const char *",
}
},
{ .nr = 176, .name = "delete_module", .num_args = 2,
.arg_types={ 
"const chat *",
"unsigned int",
}
},
{ .nr = 177, .name = "get_kernel_syms", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 178, .name = "query_module", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 179, .name = "quotactl", .num_args = 4,
.arg_types={ 
"unsigned int",
"const char *",
"qid_t",
"void *",
}
},
{ .nr = 180, .name = "nfsservctl", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 181, .name = "getpmsg", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 182, .name = "putpmsg", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 183, .name = "afs_syscall", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 184, .name = "tuxcall", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 185, .name = "security", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 186, .name = "gettid", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 187, .name = "readahead", .num_args = 3,
.arg_types={ 
"int",
"loff_t",
"size_t",
}
},
{ .nr = 188, .name = "setxattr", .num_args = 5,
.arg_types={ 
"const char *",
"const char *",
"const void *",
"size_t",
"int",
}
},
{ .nr = 189, .name = "lsetxattr", .num_args = 5,
.arg_types={ 
"const char *",
"const char *",
"const void *",
"size_t",
"int",
}
},
{ .nr = 190, .name = "fsetxattr", .num_args = 5,
.arg_types={ 
"int",
"const char *",
"const void *",
"size_t",
"int",
}
},
{ .nr = 191, .name = "getxattr", .num_args = 4,
.arg_types={ 
"const char *",
"const char *",
"void *",
"size_t",
}
},
{ .nr = 192, .name = "lgetxattr", .num_args = 4,
.arg_types={ 
"const char *",
"const char *",
"void *",
"size_t",
}
},
{ .nr = 193, .name = "fgetxattr", .num_args = 4,
.arg_types={ 
"int",
"const har *",
"void *",
"size_t",
}
},
{ .nr = 194, .name = "listxattr", .num_args = 3,
.arg_types={ 
"const char *",
"char *",
"size_t",
}
},
{ .nr = 195, .name = "llistxattr", .num_args = 3,
.arg_types={ 
"const char *",
"char *",
"size_t",
}
},
{ .nr = 196, .name = "flistxattr", .num_args = 3,
.arg_types={ 
"int",
"char *",
"size_t",
}
},
{ .nr = 197, .name = "removexattr", .num_args = 2,
.arg_types={ 
"const char *",
"const char *",
}
},
{ .nr = 198, .name = "lremovexattr", .num_args = 2,
.arg_types={ 
"const char *",
"const char *",
}
},
{ .nr = 199, .name = "fremovexattr", .num_args = 2,
.arg_types={ 
"int",
"const char *",
}
},
{ .nr = 200, .name = "tkill", .num_args = 2,
.arg_types={ 
"pid_t",
"ing",
}
},
{ .nr = 201, .name = "time", .num_args = 1,
.arg_types={ 
"time_t *",
}
},
{ .nr = 202, .name = "futex", .num_args = 6,
.arg_types={ 
"u32 *",
"int",
"u32",
"struct timespec *",
"u32 *",
"u32",
}
},
{ .nr = 203, .name = "sched_setaffinity", .num_args = 3,
.arg_types={ 
"pid_t",
"unsigned int",
"unsigned long *",
}
},
{ .nr = 204, .name = "sched_getaffinity", .num_args = 3,
.arg_types={ 
"pid_t",
"unsigned int",
"unsigned long *",
}
},
{ .nr = 205, .name = "set_thread_area", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 206, .name = "io_setup", .num_args = 2,
.arg_types={ 
"unsigned",
"aio_context_t *",
}
},
{ .nr = 207, .name = "io_destroy", .num_args = 1,
.arg_types={ 
"aio_context_t",
}
},
{ .nr = 208, .name = "io_getevents", .num_args = 4,
.arg_types={ 
"aio_context_t",
"long",
"long",
"struct io_event *",
}
},
{ .nr = 209, .name = "io_submit", .num_args = 3,
.arg_types={ 
"aio_context_t",
"long",
"struct iocb *",
}
},
{ .nr = 210, .name = "io_cancel", .num_args = 3,
.arg_types={ 
"aio_context_t",
"struct iocb *",
"struct io_event *",
}
},
{ .nr = 211, .name = "get_thread_area", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 212, .name = "lookup_dcookie", .num_args = 3,
.arg_types={ 
"u64",
"long",
"long",
}
},
{ .nr = 213, .name = "epoll_create", .num_args = 1,
.arg_types={ 
"int",
}
},
{ .nr = 214, .name = "epoll_ctl_old", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 215, .name = "epoll_wait_old", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 216, .name = "remap_file_pages", .num_args = 5,
.arg_types={ 
"unsigned long",
"unsigned long",
"unsigned long",
"unsigned long",
"unsigned long",
}
},
{ .nr = 217, .name = "getdents64", .num_args = 3,
.arg_types={ 
"unsigned int",
"struct linux_dirent64 *",
"unsigned int",
}
},
{ .nr = 218, .name = "set_tid_address", .num_args = 1,
.arg_types={ 
"int *",
}
},
{ .nr = 219, .name = "restart_syscall", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 220, .name = "semtimedop", .num_args = 4,
.arg_types={ 
"int",
"struct sembuf *",
"unsigned",
"const struct timespec *",
}
},
{ .nr = 221, .name = "fadvise64", .num_args = 4,
.arg_types={ 
"int",
"loff_t",
"size_t",
"int",
}
},
{ .nr = 222, .name = "timer_create", .num_args = 3,
.arg_types={ 
"const clockid_t",
"struct sigevent *",
"timer_t *",
}
},
{ .nr = 223, .name = "timer_settime", .num_args = 4,
.arg_types={ 
"timer_t",
"int",
"const struct itimerspec *",
"struct itimerspec *",
}
},
{ .nr = 224, .name = "timer_gettime", .num_args = 2,
.arg_types={ 
"timer_t",
"struct itimerspec *",
}
},
{ .nr = 225, .name = "timer_getoverrun", .num_args = 1,
.arg_types={ 
"timer_t",
}
},
{ .nr = 226, .name = "timer_delete", .num_args = 1,
.arg_types={ 
"timer_t",
}
},
{ .nr = 227, .name = "clock_settime", .num_args = 2,
.arg_types={ 
"const clockid_t",
"const struct timespec *",
}
},
{ .nr = 228, .name = "clock_gettime", .num_args = 2,
.arg_types={ 
"const clockid_t",
"struct timespec *",
}
},
{ .nr = 229, .name = "clock_getres", .num_args = 2,
.arg_types={ 
"const clockid_t",
"struct timespec *",
}
},
{ .nr = 230, .name = "clock_nanosleep", .num_args = 4,
.arg_types={ 
"const clockid_t",
"int",
"const struct timespec *",
"struct timespec *",
}
},
{ .nr = 231, .name = "exit_group", .num_args = 1,
.arg_types={ 
"int",
}
},
{ .nr = 232, .name = "epoll_wait", .num_args = 4,
.arg_types={ 
"int",
"struct epoll_event *",
"int",
"int",
}
},
{ .nr = 233, .name = "epoll_ctl", .num_args = 4,
.arg_types={ 
"int",
"int",
"int",
"struct epoll_event *",
}
},
{ .nr = 234, .name = "tgkill", .num_args = 3,
.arg_types={ 
"pid_t",
"pid_t",
"int",
}
},
{ .nr = 235, .name = "utimes", .num_args = 2,
.arg_types={ 
"char *",
"struct timeval *",
}
},
{ .nr = 236, .name = "vserver", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 237, .name = "mbind", .num_args = 6,
.arg_types={ 
"unsigned long",
"unsigned long",
"unsigned long",
"unsigned long *",
"unsigned long",
"unsigned",
}
},
{ .nr = 238, .name = "set_mempolicy", .num_args = 3,
.arg_types={ 
"int",
"unsigned long *",
"unsigned long",
}
},
{ .nr = 239, .name = "get_mempolicy", .num_args = 5,
.arg_types={ 
"int *",
"unsigned long *",
"unsigned long",
"unsigned long",
"unsigned long",
}
},
{ .nr = 240, .name = "mq_open", .num_args = 4,
.arg_types={ 
"const char *",
"int",
"mode_t",
"struct mq_attr *",
}
},
{ .nr = 241, .name = "mq_unlink", .num_args = 1,
.arg_types={ 
"const char *",
}
},
{ .nr = 242, .name = "mq_timedsend", .num_args = 5,
.arg_types={ 
"mqd_t",
"const char *",
"size_t",
"unsigned int",
"const stuct timespec *",
}
},
{ .nr = 243, .name = "mq_timedreceive", .num_args = 5,
.arg_types={ 
"mqd_t",
"char *",
"size_t",
"unsigned int *",
"const struct timespec *",
}
},
{ .nr = 244, .name = "mq_notify", .num_args = 2,
.arg_types={ 
"mqd_t",
"const struct sigevent *",
}
},
{ .nr = 245, .name = "mq_getsetattr", .num_args = 3,
.arg_types={ 
"mqd_t",
"const struct mq_attr *",
"struct mq_attr *",
}
},
{ .nr = 246, .name = "kexec_load", .num_args = 4,
.arg_types={ 
"unsigned long",
"unsigned long",
"struct kexec_segment *",
"unsigned long",
}
},
{ .nr = 247, .name = "waitid", .num_args = 5,
.arg_types={ 
"int",
"pid_t",
"struct siginfo *",
"int",
"struct rusage *",
}
},
{ .nr = 248, .name = "add_key", .num_args = 4,
.arg_types={ 
"const char *",
"const char *",
"const void *",
"size_t",
}
},
{ .nr = 249, .name = "request_key", .num_args = 4,
.arg_types={ 
"const char *",
"const char *",
"const char *",
"key_serial_t",
}
},
{ .nr = 250, .name = "keyctl", .num_args = 5,
.arg_types={ 
"int",
"unsigned long",
"unsigned long",
"unsigned long",
"unsigned long",
}
},
{ .nr = 251, .name = "ioprio_set", .num_args = 3,
.arg_types={ 
"int",
"int",
"int",
}
},
{ .nr = 252, .name = "ioprio_get", .num_args = 2,
.arg_types={ 
"int",
"int",
}
},
{ .nr = 253, .name = "inotify_init", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 254, .name = "inotify_add_watch", .num_args = 3,
.arg_types={ 
"int",
"const char *",
"u32",
}
},
{ .nr = 255, .name = "inotify_rm_watch", .num_args = 2,
.arg_types={ 
"int",
"__s32",
}
},
{ .nr = 256, .name = "migrate_pages", .num_args = 4,
.arg_types={ 
"pid_t",
"unsigned long",
"const unsigned long *",
"const unsigned long *",
}
},
{ .nr = 257, .name = "openat", .num_args = 4,
.arg_types={ 
"int",
"const char *",
"int",
"int",
}
},
{ .nr = 258, .name = "mkdirat", .num_args = 3,
.arg_types={ 
"int",
"const char *",
"int",
}
},
{ .nr = 259, .name = "mknodat", .num_args = 4,
.arg_types={ 
"int",
"const char *",
"int",
"unsigned",
}
},
{ .nr = 260, .name = "fchownat", .num_args = 5,
.arg_types={ 
"int",
"const char *",
"uid_t",
"gid_t",
"int",
}
},
{ .nr = 261, .name = "futimesat", .num_args = 3,
.arg_types={ 
"int",
"const char *",
"struct timeval *",
}
},
{ .nr = 262, .name = "newfstatat", .num_args = 4,
.arg_types={ 
"int",
"const char *",
"struct stat *",
"int",
}
},
{ .nr = 263, .name = "unlinkat", .num_args = 3,
.arg_types={ 
"int",
"const char *",
"int",
}
},
{ .nr = 264, .name = "renameat", .num_args = 4,
.arg_types={ 
"int",
"const char *",
"int",
"const char *",
}
},
{ .nr = 265, .name = "linkat", .num_args = 5,
.arg_types={ 
"int",
"const char *",
"int",
"const char *",
"int",
}
},
{ .nr = 266, .name = "symlinkat", .num_args = 3,
.arg_types={ 
"const char *",
"int",
"const char *",
}
},
{ .nr = 267, .name = "readlinkat", .num_args = 4,
.arg_types={ 
"int",
"const char *",
"char *",
"int",
}
},
{ .nr = 268, .name = "fchmodat", .num_args = 3,
.arg_types={ 
"int",
"const char *",
"mode_t",
}
},
{ .nr = 269, .name = "faccessat", .num_args = 3,
.arg_types={ 
"int",
"const char *",
"int",
}
},
{ .nr = 270, .name = "pselect6", .num_args = 6,
.arg_types={ 
"int",
"fd_set *",
"fd_set *",
"fd_set *",
"struct timespec *",
"void *",
}
},
{ .nr = 271, .name = "ppoll", .num_args = 5,
.arg_types={ 
"struct pollfd *",
"unsigned int",
"struct timespec *",
"const sigset_t *",
"size_t",
}
},
{ .nr = 272, .name = "unshare", .num_args = 1,
.arg_types={ 
"unsigned long",
}
},
{ .nr = 273, .name = "set_robust_list", .num_args = 2,
.arg_types={ 
"struct robust_list_head *",
"size_t",
}
},
{ .nr = 274, .name = "get_robust_list", .num_args = 3,
.arg_types={ 
"int",
"struct robust_list_head *",
"size_t *",
}
},
{ .nr = 275, .name = "splice", .num_args = 6,
.arg_types={ 
"int",
"loff_t *",
"int",
"loff_t *",
"size_t",
"unsigned int",
}
},
{ .nr = 276, .name = "tee", .num_args = 4,
.arg_types={ 
"int",
"int",
"size_t",
"unsigned int",
}
},
{ .nr = 277, .name = "sync_file_range", .num_args = 4,
.arg_types={ 
"long",
"loff_t",
"loff_t",
"long",
}
},
{ .nr = 278, .name = "vmsplice", .num_args = 4,
.arg_types={ 
"int",
"const struct iovec *",
"unsigned long",
"unsigned int",
}
},
{ .nr = 279, .name = "move_pages", .num_args = 6,
.arg_types={ 
"pid_t",
"unsigned long",
"const void *",
"const int *",
"int *",
"int",
}
},
{ .nr = 280, .name = "utimensat", .num_args = 4,
.arg_types={ 
"int",
"const char *",
"struct timespec *",
"int",
}
},
{ .nr = 281, .name = "epoll_pwait", .num_args = 6,
.arg_types={ 
"int",
"struct epoll_event *",
"int",
"int",
"const sigset_t *",
"size_t",
}
},
{ .nr = 282, .name = "signalfd", .num_args = 3,
.arg_types={ 
"int",
"sigset_t *",
"size_t",
}
},
{ .nr = 283, .name = "timerfd_create", .num_args = 2,
.arg_types={ 
"int",
"int",
}
},
{ .nr = 284, .name = "eventfd", .num_args = 1,
.arg_types={ 
"unsigned int",
}
},
{ .nr = 285, .name = "fallocate", .num_args = 4,
.arg_types={ 
"long",
"long",
"loff_t",
"loff_t",
}
},
{ .nr = 286, .name = "timerfd_settime", .num_args = 4,
.arg_types={ 
"int",
"int",
"const struct itimerspec *",
"struct itimerspec *",
}
},
{ .nr = 287, .name = "timerfd_gettime", .num_args = 2,
.arg_types={ 
"int",
"struct itimerspec *",
}
},
{ .nr = 288, .name = "accept4", .num_args = 4,
.arg_types={ 
"int",
"struct sockaddr *",
"int *",
"int",
}
},
{ .nr = 289, .name = "signalfd4", .num_args = 4,
.arg_types={ 
"int",
"sigset_t *",
"size_t",
"int",
}
},
{ .nr = 290, .name = "eventfd2", .num_args = 2,
.arg_types={ 
"unsigned int",
"int",
}
},
{ .nr = 291, .name = "epoll_create1", .num_args = 1,
.arg_types={ 
"int",
}
},
{ .nr = 292, .name = "dup3", .num_args = 3,
.arg_types={ 
"unsigned int",
"unsigned int",
"int",
}
},
{ .nr = 293, .name = "pipe2", .num_args = 2,
.arg_types={ 
"int *",
"int",
}
},
{ .nr = 294, .name = "inotify_init1", .num_args = 1,
.arg_types={ 
"int",
}
},
{ .nr = 295, .name = "preadv", .num_args = 5,
.arg_types={ 
"unsigned long",
"const struct iovec *",
"unsigned long",
"unsigned long",
"unsigned long",
}
},
{ .nr = 296, .name = "pwritev", .num_args = 5,
.arg_types={ 
"unsigned long",
"const struct iovec *",
"unsigned long",
"unsigned long",
"unsigned long",
}
},
{ .nr = 297, .name = "rt_tgsigqueueinfo", .num_args = 4,
.arg_types={ 
"pid_t",
"pid_t",
"int",
"siginfo_t *",
}
},
{ .nr = 298, .name = "perf_event_open", .num_args = 5,
.arg_types={ 
"struct perf_event_attr *",
"pid_t",
"int",
"int",
"unsigned long",
}
},
{ .nr = 299, .name = "recvmmsg", .num_args = 5,
.arg_types={ 
"int",
"struct msghdr *",
"unsigned int",
"unsigned int",
"struct timespec *",
}
},
{ .nr = 300, .name = "fanotify_init", .num_args = 2,
.arg_types={ 
"unsigned int",
"unsigned int",
}
},
{ .nr = 301, .name = "fanotify_mark", .num_args = 5,
.arg_types={ 
"long",
"long",
"__u64",
"long",
"long",
}
},
{ .nr = 302, .name = "prlimit64", .num_args = 4,
.arg_types={ 
"pid_t",
"unsigned int",
"const struct rlimit64 *",
"struct rlimit64 *",
}
},
{ .nr = 303, .name = "name_to_handle_at", .num_args = 5,
.arg_types={ 
"int",
"const char *",
"struct file_handle *",
"int *",
"int",
}
},
{ .nr = 304, .name = "open_by_handle_at", .num_args = 5,
.arg_types={ 
"int",
"const char *",
"struct file_handle *",
"int *",
"int",
}
},
{ .nr = 305, .name = "clock_adjtime", .num_args = 2,
.arg_types={ 
"clockid_t",
"struct timex *",
}
},
{ .nr = 306, .name = "syncfs", .num_args = 1,
.arg_types={ 
"int",
}
},
{ .nr = 307, .name = "sendmmsg", .num_args = 4,
.arg_types={ 
"int",
"struct mmsghdr *",
"unsigned int",
"unsigned int",
}
},
{ .nr = 308, .name = "setns", .num_args = 2,
.arg_types={ 
"int",
"int",
}
},
{ .nr = 309, .name = "getcpu", .num_args = 3,
.arg_types={ 
"unsigned *",
"unsigned *",
"struct getcpu_cache *",
}
},
{ .nr = 310, .name = "process_vm_readv", .num_args = 6,
.arg_types={ 
"pid_t",
"const struct iovec *",
"unsigned long",
"const struct iovec *",
"unsigned long",
"unsigned long",
}
},
{ .nr = 311, .name = "process_vm_writev", .num_args = 6,
.arg_types={ 
"pid_t",
"const struct iovec *",
"unsigned long",
"const struct iovcc *",
"unsigned long",
"unsigned long",
}
},
{ .nr = 312, .name = "kcmp", .num_args = 5,
.arg_types={ 
"pid_t",
"pid_t",
"int",
"unsigned long",
"unsigned long",
}
},
{ .nr = 313, .name = "finit_module", .num_args = 3,
.arg_types={ 
"int",
"const char __user *",
"int",
}
},
{ .nr = 314, .name = "sched_setattr", .num_args = 3,
.arg_types={ 
"pid_t",
"struct sched_attr __user *",
"unsigned int",
}
},
{ .nr = 315, .name = "sched_getattr", .num_args = 4,
.arg_types={ 
"pid_t",
"struct sched_attr __user *",
"unsigned int",
"unsigned int",
}
},
{ .nr = 316, .name = "renameat2", .num_args = 5,
.arg_types={ 
"int",
"const char __user *",
"int",
"const char __user *",
"unsigned int",
}
},
{ .nr = 317, .name = "seccomp", .num_args = 3,
.arg_types={ 
"unsigned int",
"unsigned int",
"const char __user *",
}
},
{ .nr = 318, .name = "getrandom", .num_args = 3,
.arg_types={ 
"char __user *",
"size_t",
"unsigned int",
}
},
{ .nr = 319, .name = "memfd_create", .num_args = 2,
.arg_types={ 
"const char __user *",
"unsigned int",
}
},
{ .nr = 320, .name = "kexec_file_load", .num_args = 5,
.arg_types={ 
"int",
"int",
"unsigned long",
"const char __user *",
"unsigned long",
}
},
{ .nr = 321, .name = "bpf", .num_args = 3,
.arg_types={ 
"int",
"union bpf_attr *",
"unsigned int",
}
},
{ .nr = 322, .name = "_execveat", .num_args = 5,
.arg_types={ 
"int",
"const char __user *",
"const char __user *const __user *",
"const char __user *const __user *",
"int",
}
},
{ .nr = 323, .name = "faultfd", .num_args = 1,
.arg_types={ 
"int",
}
},
{ .nr = 324, .name = "arrier", .num_args = 2,
.arg_types={ 
"int",
"int",
}
},
{ .nr = 325, .name = "k2", .num_args = 3,
.arg_types={ 
"unsigned long",
"size_t",
"int",
}
},
{ .nr = 326, .name = "_file_range", .num_args = 6,
.arg_types={ 
"int",
"loff_t __user *",
"int",
"loff_t __user *",
"size_t",
"unsigned int",
}
},
{ .nr = 327, .name = "dv2", .num_args = 6,
.arg_types={ 
"unsigned long",
"const struct iovec __user *",
"unsigned long",
"unsigned long",
"unsigned long",
"int",
}
},
{ .nr = 328, .name = "tev2", .num_args = 6,
.arg_types={ 
"unsigned long",
"const struct iovec __user *",
"unsigned long",
"unsigned long",
"unsigned long",
"int",
}
},
{ .nr = 329, .name = "_mprotect", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 330, .name = "_alloc", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 331, .name = "_free", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 332, .name = "x", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 333, .name = "getevents", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 334, .name = "", .num_args = 0,
.arg_types={ 
}
},
{ .nr = 335, .name = "_mprotect", .num_args = 0,
.arg_types={ 
}
},

};


unordered_map<string, int> syscall_name_2_nr;
int SyscallNameToNr(const string & name) {
  static bool inited = false;
  if (!inited) {
    for (size_t i=0; i<(sizeof(syscall_infos) / sizeof(SyscallInfo)); i++) {
      SyscallInfo & info = syscall_infos[i];
      syscall_name_2_nr[info.name] = info.nr;
    }
    inited = true;
  }
  if (syscall_name_2_nr.count(name))
    return syscall_name_2_nr[name];
  else
    return -1;
}