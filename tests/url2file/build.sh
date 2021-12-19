#! /bin/bash

rm debug.log ptrace_out.c seccomp_out.c sandbox
../../demo url2file_dsl.p ptrace_out.c seccomp_out.c
gcc ptrace_out.c seccomp_out.c helpers.c -o sandbox
