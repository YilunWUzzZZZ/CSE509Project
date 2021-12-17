#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>
#include "helpers.h"

#define MAX_STRING_LEN  40960
#define X32_SYSCALL_BIT 0x40000000U
#define X32_MASK 0xFFFFFFFFULL
int DEBUG = 0;

#define debug_print(...) do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)


int deny = 1;
int reg_modified  = 0;


#define DENY() {deny = 1; goto out;}
#define ALLOW() {deny = 0; goto out;}


#define SET_ARG(idx, new_val) {arguments[idx] = new_val; reg_modified=1;}
#define GET_ARG(idx) (arguments[idx])



char * ptrace_copy_out_mem(int child, void * tracee_ptr, int copy_size) {
    int n_words = copy_size / sizeof(void*);
    n_words = (n_words % sizeof(void*)) ? n_words + 1 : n_words;
    long * new_buf = (long*)malloc(n_words * sizeof(void*));
    for (int i=0; i<n_words; i++) {
        new_buf[i] = ptrace(PTRACE_PEEKDATA,
                            child, tracee_ptr,
                            NULL);
        tracee_ptr += sizeof(void*);
    }
    return (char*)new_buf;
}

void ptrace_copy_back_mem(int child, void * tracee_ptr, void * tracer_ptr, int copy_size) {
    int n_words = copy_size / sizeof(void*);
    n_words = (n_words % sizeof(void*)) ? n_words + 1 : n_words;
    long * buf = (long*)tracer_ptr;
    for (int i=0; i<n_words; i++) {
        ptrace(PTRACE_POKEDATA,
                            child, tracee_ptr,
                            buf[i]);
        tracee_ptr += sizeof(void*);
    }
}

char * ptrace_copy_out_string(int child, void * tracee_ptr) {
    char * tracee_addr = tracee_ptr;
    union {
        long val;
        char word[sizeof(void*)];
    } data;
    int cnt = 0;
    char * str = malloc(MAX_STRING_LEN+1);
    int string_end = 0;
    while (cnt < MAX_STRING_LEN) {
        data.val = ptrace(PTRACE_PEEKDATA,
                            child, tracee_addr,
                            NULL);
        memcpy(&str[cnt], data.word, sizeof(void*));

        for (int i=0; i < sizeof(void*); i++) {
            cnt++;
            tracee_addr++;
            if (!data.word[i]) {
                string_end = 1;
                break;
            } 
        }

        if (string_end) {
            break;
        }
    }
    str[MAX_STRING_LEN] = 0;
    return str;
}

void ptrace_copy_back_string(int child, char * tracer_ptr, void * tracee_ptr) {
    int str_len = strlen(tracer_ptr) + 1;
    int n_words = str_len / sizeof(void*);
    int cnt = 0;
    int remain = str_len % sizeof(void*);
    long * new_ptr = (long*)tracer_ptr;
    long * target_ptr = (long*)tracee_ptr;

    while(cnt < n_words) {
        ptrace(PTRACE_POKEDATA, child,
               target_ptr, *new_ptr);

        cnt += 1;
        new_ptr += 1;
        target_ptr += 1;
    }
    
    if(remain) {
        long data = 0;
        memcpy((void*)&data, (void*)new_ptr, remain);
        ptrace(PTRACE_POKEDATA, child,
               target_ptr, data);
    }
    free(tracer_ptr);
}




int main(int argc, char ** argv)
{   
    /* insert */void * jmp_table[] = {}; void * start_table[]  = {};
    pid_t child;
    pid_t direct_child;
    int first_fork = 1;
    long params[3];
    int status;
    int syscall_code = -1;
    int terminated = 0;


    struct user_regs_struct regs;
    if (argc < 3) {
        printf("usage: tracer [seccomp] [prisoner_app] ... \n");
        exit(-1);
    }
    char * bin = argv[1];
    char *  args[argc];

    for (int i=0; i<argc-1; i++) {
        args[i] = argv[i+1];
    }
    args[argc-1] = NULL;

    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        if (execvp(bin, args)) {
            perror("execv:");
        }
    } else {
        direct_child = child;
        while(1) {
            child = wait(&status);
            
            if( child == -1 || (WIFEXITED(status) && child == direct_child))
                break;
            
            if (first_fork && child == direct_child && WIFSTOPPED(status)) {
                first_fork = 0;
                // auto-attach the whole process tree
                printf("Setting up ptrace\n");
                ptrace(PTRACE_SETOPTIONS, child, NULL,  PTRACE_O_EXITKILL| PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | 
                PTRACE_O_TRACECLONE | PTRACE_O_TRACESECCOMP);
                ptrace(PTRACE_CONT, child, NULL, NULL);
                continue;
            }

            if ( !(status>>8 == (SIGTRAP | (PTRACE_EVENT_SECCOMP<<8)))) {
              ptrace(PTRACE_CONT, child, NULL, NULL);
              continue;
            } else {
              printf("Received request from seccomp\n");
            }

            unsigned long seccomp_ret_data;
            ptrace(PTRACE_GETREGS, child, NULL, &regs);
            // Get event message to locate the exact handling logic
            ptrace(PTRACE_GETEVENTMSG, child, NULL, &seccomp_ret_data);
            unsigned long long arguments[6];
            reg_modified=0;
            if (regs.orig_rax & X32_SYSCALL_BIT) {
                arguments[0] = regs.rbx & X32_MASK;
                arguments[1] = regs.rcx & X32_MASK;
                arguments[2] = regs.rdx & X32_MASK;
                arguments[3] = regs.rsi & X32_MASK;
                arguments[4] = regs.rdi & X32_MASK;
                arguments[5] = regs.rbp & X32_MASK;
            } else {
                arguments[0] = regs.rdi;
                arguments[1] = regs.rsi;
                arguments[2] = regs.rdx;
                arguments[3] = regs.r10;
                arguments[4] = regs.r8;
                arguments[5] = regs.r9;
            }
            goto *start_table[seccomp_ret_data];
            /* insert */      
   
out:     
            if (deny) {
              printf("Killing offending process %d\n", child);
              kill(child, SIGKILL);
              continue;
            } else {
              if (reg_modified) {
                if (regs.orig_rax & X32_SYSCALL_BIT) {
                    regs.rbx = arguments[0];
                    regs.rcx = arguments[1];
                    regs.rdx = arguments[2];
                    regs.rsi = arguments[3];
                    regs.rdi = arguments[4];
                    regs.rbp = arguments[5];
                } else {
                    regs.rdi = arguments[0];
                    regs.rsi = arguments[1];
                    regs.rdx = arguments[2];
                    regs.r10 = arguments[3];
                    regs.r8 = arguments[4];
                    regs.r9 = arguments[5];
                }
                ptrace(PTRACE_SETREGS, child, NULL, &regs);
              }
              ptrace(PTRACE_CONT, child,
                      NULL, NULL);
            }
       }
   }
   return 0;
}