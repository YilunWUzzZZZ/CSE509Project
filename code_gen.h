#ifndef CODE_GEN_H
#define CODE_GEN_H
#include <unordered_map>
#include "IR.h"
#include "IR_lifter.h"

typedef unordered_map<BasicBlock*, Instruction*> PtraceBasicBlock;
#define RET_TO_PTRACE 3
#define RET_TO_USER 4

enum BPF_REGS {
  A = 1,
  X,
  M0,
};

struct BPF_Filter {
  string opcode;
  string k;
  string jt;
  string jf;
  string label;
  string type;
  int pc;
};

void ColorCode(vector<BasicBlock*> & CFG, PtraceBasicBlock &ptrace_blocks, PtraceBasicBlock &ptrace_only_blocks);
void GenBPFIR(vector<BasicBlock*> & CFG, PtraceBasicBlock &ptrace_only_blocks, list<Instruction*> & BPF_IR, IRLifter & lifter);
void GenPtraceCode(vector<BasicBlock*> & CFG, PtraceBasicBlock &ptrace_blocks, IRLifter & lifter);
list<BPF_Filter> * BPFCodeGen(list<Instruction*> & BPF_IR, unordered_map<string, int> *arg_index, IRLifter & lifter);
string StringfyBPF_Filter(const BPF_Filter & filter);
void BPFTransformLabels(list<BPF_Filter> & filters);
void GenPtracePrologueAndEpilogue(int syscall_nr, vector<string> & args, PtraceBasicBlock &ptrace_blocks,
                                  vector<string> & prologue, vector<string> & epilogue);
#endif