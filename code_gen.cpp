#include "IR_gen.h"
#include "IR.h"
#include "IR_lifter.h"
#include "code_gen.h"
#include <set>
#include <list>

static bool InstRequiresPtrace(Instruction * inst) {
  if (isa<CallInst>(inst) || isa<StoreInst>(inst)) {
    return true;
  } 
  return false;
}

static void GetPtraceOnlyBB(PtraceBasicBlock &ptrace_blocks, PtraceBasicBlock &ptrace_only_blocks) {
  bool changed;
  list<BasicBlock*> candidates;
  for (auto p: ptrace_blocks) {
    candidates.push_back(p.first);
  }
  do {
    changed = false;
    for (auto bb : candidates) {
      if (ptrace_only_blocks.count(bb)) {
        continue;
      }
      bool dominated = true;
      for (auto parent : bb->GetParents()) {
        if (!ptrace_only_blocks.count(parent)) {
          dominated = false;
          break;
        }
      }
      // is this BB dominated by itself?
      if (!dominated) {
        if (bb->GetParents().size() <= 1) {
          dominated = true;
        }
      }

      if (dominated) {
        ptrace_only_blocks[bb] = ptrace_blocks[bb];
        changed = true;
      }

    }
  } while (changed);
}

void ColorCode(vector<BasicBlock*> & CFG, PtraceBasicBlock &ptrace_blocks, PtraceBasicBlock &ptrace_only_blocks) {
  // BBs in the CFG are in program order.
  // There is no backward jump
  for (auto bb : CFG) { 
    bool bb_is_ptrace = false;
    for (auto parent : bb->GetParents()) {
      if (ptrace_blocks.count(parent)) {
        ptrace_blocks[bb] = bb->FirstInst();
        bb_is_ptrace = true;
        break;
      }
    }
    if (bb_is_ptrace) {
      continue;
    }
    // no parent requires ptrace
    for (auto i : bb->Insts()) {
      if (InstRequiresPtrace(i)) {
        ptrace_blocks[bb] = i;
        break;
      }
    }
  }
  GetPtraceOnlyBB(ptrace_blocks, ptrace_only_blocks);
}

unordered_map<Instruction*, string> bpfret_to_string;

void GenBPFIR(vector<BasicBlock*> & CFG, PtraceBasicBlock &ptrace_only_blocks, list<Instruction*> & BPF_IR, IRLifter & lifter)
{
  set<BasicBlock*> ptrace_targets;
  for (auto bb : CFG) {
    if (!ptrace_only_blocks.count(bb)) {
      // contains only BPF code
      BPF_IR.push_back(new LabelInst(bb->GetLabel()));
      for (auto i : bb->Insts()) {
        BPF_IR.push_back(i);
      }
    } else if(ptrace_only_blocks.count(bb) && ptrace_only_blocks[bb] != bb->FirstInst()) {
      BPF_IR.push_back(new LabelInst(bb->GetLabel()));
      auto end = ptrace_only_blocks[bb];
      for (auto i : bb->Insts()) {
        if (i == end) {
          // transition point
          Instruction * ret = new ReturnInst(new Immediate<int>(RET_TO_PTRACE));
          string l = lifter.GenInstLabel(end);
          bpfret_to_string[ret] = l;
          BPF_IR.push_back(ret);
          break;
        }
        BPF_IR.push_back(i);
      } 
      continue;
    } else if (ptrace_targets.count(bb)) {
      BPF_IR.push_back(new LabelInst(bb->GetLabel()));
      Instruction * ret = new ReturnInst(new Immediate<int>(RET_TO_PTRACE));
      bpfret_to_string[ret] = bb->GetLabel();
      BPF_IR.push_back(ret);
      continue;
    } else {
      // This is a ptrace-only basic block whose parent is also a ptrace only basic block
      // Just ignore it for BPF
      continue;
    }

    for (auto child : bb->GetChildren()) {
      if (ptrace_only_blocks.count(child)) {
        ptrace_targets.insert(child);
      }
    }
  }
}

void GenPtraceCode(vector<BasicBlock*> & CFG, PtraceBasicBlock &ptrace_blocks, IRLifter & lifter) {
  for (auto bb: CFG) {
    if (!ptrace_blocks.count(bb)) {
      continue;
    }
    Instruction * start = ptrace_blocks[bb];
    if (start != bb->FirstInst()) {
      auto it = bb->Insts().begin();
      while (*it != start) {
        ++it;
      }
      lifter.LiftInst(start, lifter.GenInstLabel(start));
      ++it;
      while (it != bb->Insts().end()) {
        lifter.LiftInst(*it, "");
        ++it;
      }
    } else {
      lifter.LiftBasicBlock(bb);
    }
  }
}