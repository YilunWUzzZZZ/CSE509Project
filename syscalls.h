#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <string>
using namespace std;

int SyscallNameToNr(const string & name);

struct SyscallInfo {
  int nr;
  string name;
  int num_args;
  string arg_types[6];
  
};

extern SyscallInfo syscall_infos[];
extern string syscall_regs[];
#endif