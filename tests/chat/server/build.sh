#! /bin/bash

rm ptrace_out.c seccomp_out.c sandbox 
../../../demo rules ptrace_out.c seccomp_out.c
gcc ptrace_out.c seccomp_out.c helpers.c -o sandbox
