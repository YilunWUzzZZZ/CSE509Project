#ifndef CODE_GEN_H
#define CODE_GEN_H
#include <unordered_map>
#include "IR.h"
#include "IR_lifter.h"

typedef unordered_map<BasicBlock*, Instruction*> PtraceBasicBlock;
#define RET_TO_PTRACE 3
#define RET_TO_USER 4

void ColorCode(vector<BasicBlock*> & CFG, PtraceBasicBlock &ptrace_blocks, PtraceBasicBlock &ptrace_only_blocks);
void GenBPFIR(vector<BasicBlock*> & CFG, PtraceBasicBlock &ptrace_only_blocks, list<Instruction*> & BPF_IR, IRLifter & lifter);
void GenPtraceCode(vector<BasicBlock*> & CFG, PtraceBasicBlock &ptrace_blocks, IRLifter & lifter);

#endif