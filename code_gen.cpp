#include "IR_gen.h"
#include "IR.h"
#include "IR_lifter.h"
#include "code_gen.h"
#include "error.h"
#include "ast.h"
#include <set>
#include <list>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include "syscalls.h"

#ifdef DEBUG_MODE
#define DENY_ACTION "SECCOMP_RET_ERRNO & (SECCOMP_RET_DATA & 66)"
#else
#define DENY_ACTION "SECCOMP_RET_KILL_PROCESS"
#endif
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

int ret_data = 0;
unordered_map<Instruction*, int> bpfret_to_ret_data;
vector<string> ret_data_to_label;

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
          printf("Special Case encountered\n");
          Instruction * ret = new ReturnInst(new Immediate<int>(RET_TO_PTRACE));
          string l = lifter.GenInstLabel(end);
          bpfret_to_ret_data[ret] = ret_data++;
          ret_data_to_label.push_back(l);
          BPF_IR.push_back(ret);
          break;
        }
        BPF_IR.push_back(i);
      } 
      continue;
    } else if (ptrace_targets.count(bb)) {
      BPF_IR.push_back(new LabelInst(bb->GetLabel()));
      Instruction * ret = new ReturnInst(new Immediate<int>(RET_TO_PTRACE));
      bpfret_to_ret_data[ret] = ret_data++;
      ret_data_to_label.push_back(bb->GetLabel());
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


void GenPtracePrologueAndEpilogue(int syscall_nr, vector<string> & args, PtraceBasicBlock &ptrace_blocks,
                                  vector<string> & prologue, vector<string> & epilogue) 
{ 
  SyscallInfo & info = syscall_infos[syscall_nr];
  set<int> string_args;
  set<int> args_used;
  set<int> args_assigned;
  unordered_map<string, int> arg_index;
  for (int i=0; i<args.size(); i++) {
    arg_index[args[i]] = i;
  }
  for (int i=0 ; i<info.num_args; i++) {
    if (info.arg_types[i].find("const char *") != string::npos) {
      string_args.insert(i);
    }
  }
  for (auto pair : ptrace_blocks) {
    auto bb = pair.first;
    for (auto inst : bb->Insts()) {
      if (isa<CallInst>(inst)) {
        CallInst * call = (CallInst*)inst;
        auto fargs = call->GetArgs();
        for (auto & arg : *fargs) {
          args_used.insert(arg_index[arg]);
        }
      } else if (isa<StoreInst>(inst)) {
        IRValue * val = inst->GetOperand(1);
        assert(isa<Memory>(val));
        int index = arg_index[val->String()];
        args_assigned.insert(index);
        IRValue * op1 = inst->GetOperand(0);
        if (isa<Memory>(op1)) {
          index = arg_index[op1->String()];
          args_used.insert(index);
        }
      } else if (isa<ArithmeticInst>(inst) || isa<BranchInst>(inst)) {
        IRValue * op1, * op2;
        op1 = inst->GetOperand(0);
        op2 = inst->GetOperand(1);
        if (op1 && isa<Memory>(op1)) {
          args_used.insert(arg_index[op1->String()]);
        }
        if (op2 && isa<Memory>(op2)) {
          args_used.insert(arg_index[op2->String()]);
        }
      } 
    }
  }

  // Extract arguments
  for (int i=0; i<info.num_args; i++) {
    if (!args_used.count(i)) {
      continue;
    }
    if (string_args.count(i)) {
      prologue.push_back("char *" + args[i] + " = ptrace_copy_out_string(child, regs." + syscall_regs[i] + ");");
    } else {
      prologue.push_back(info.arg_types[i] +  " " +  args[i] + " = regs." + syscall_regs[i] + ";");
    }
  }

  // copy back arguments
  bool non_mem_args_changed = false;
  for (int i=0; i<info.num_args; i++) {
    if (!args_assigned.count(i)) {
      continue;
    }
    if (string_args.count(i)) {
      epilogue.push_back("ptrace_copy_back_string(child, " + args[i] + ", regs." + syscall_regs[i] + ");");
    } else {
      epilogue.push_back("regs." + syscall_regs[i] + " = " + args[i] + ";");
      non_mem_args_changed = true;
    }
  }
  if (non_mem_args_changed) {
    epilogue.push_back("ptrace(PTRACE_SETREGS, child, NULL, &regs);");
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

static string bpf_opcodes[] = {
    "BPF_ERROR", "BPF_ADD", "BPF_SUB", "BPF_MUL", "BPF_DIV", "BPF_MOD", 
    "BPF_JEQ", "BPF_JEQ", "BPF_JGT", "BPF_JGT", "BPF_JGE", "BPF_JGE",
    "BPF_ERROR", "BPF_ERROR", "BPF_ERROR", 
    "BPF_NEG", "BPF_AND", "BPF_OR", "BPF_XOR", 
    "BPF_LSH", "BPF_RSH",
    "INVALID",
};

static inline bool NeedSwapTarget(OpNode::OpCode jmp_type) {
  return jmp_type == OpNode::LE || jmp_type == OpNode::LT || jmp_type == OpNode::NE;
}

static string GenK_Field(IRValue * val, unordered_map<string, int> *arg_index) {
  if (isa<Immediate<int>>(val)) {
      return val->String();
  } else if (isa<Memory>(val)) {
      return "offsetof(struct seccomp_data, args) + 8*" + to_string((*arg_index)[val->String()]);
  } else {
    internalErr("Unknown Case");
    return "";
  }
}

static void LoadArgumentOrInt(IRValue * val, unordered_map<string, int> *arg_index, list<BPF_Filter> * code) {
  
  if (isa<Memory>(val)) {
    string k_field = "offsetof(struct seccomp_data, args) + 8*" + to_string((*arg_index)[val->String()]);
    code->push_back({.opcode = "BPF_LD | BPF_W | BPF_ABS", .k=k_field, .type="BPF_STMT"} );
  } else if (isa<Immediate<int>>(val)) {
    code->push_back({.opcode = "BPF_LD | BPF_W | BPF_K", .k=val->String(), .type="BPF_STMT"});
  }
}

inline static string GetAddressMode(IRValue * val) {
  if (isa<Memory>(val)) {
    return "BPF_ABS";
  } else if (isa<Immediate<int>>(val)) {
    return "BPF_K";
  } else {
    internalErr("Unexpected case");
    return "BPF_ERR";
  }
}

// if (load_mem) {
//     BPF_Filter filter = {.opcode= ld_type + " | BPF_MEM | BPF_W", .k=0, .type="BPF_STMT"};
//     code->push_back(filter);
//   }

list<BPF_Filter> * BPFCodeGen(list<Instruction*> & BPF_IR, unordered_map<string, int> *arg_index, IRLifter & lifter) {
  auto end = BPF_IR.end();
  auto it = BPF_IR.begin();


  list<BPF_Filter> * code = new list<BPF_Filter>();

  IRValue *A_val = nullptr;
  IRValue *M0_val = nullptr;
  BPF_Filter filter;
  ++it; // skip start label
  while (it != end) {
    Instruction * inst = *it++;
    // cerr << "Generating " << inst->String() << "\n";
    if (isa<ArithmeticInst>(inst)) {
      ArithmeticInst * arith = (ArithmeticInst*)(inst);
      IRValue * op1 = arith->GetOperand(0);
      IRValue * op2 = arith->GetOperand(1);
      if(A_val && A_val != op1 && A_val != op2) {
        filter = {.opcode="BPF_ST |  BPF_W", .k="0", .type="BPF_STMT"};
        code->push_back(filter);
        M0_val = A_val;
        A_val = nullptr;
        
      }
      if (isa<Instruction>(op1) && op2 && isa<Instruction>(op2)) {
        assert(op1 == M0_val || op2 == M0_val);
        filter = {.opcode=  "BPF_LDX | BPF_MEM | BPF_W", .k="0", .type="BPF_STMT"};
        M0_val = nullptr;
        code->push_back(filter);
        filter = {.opcode="BPF_ALU | BPF_X | " + bpf_opcodes[arith->GetOpcode()], 
                             .type="BPF_STMT"};
        code->push_back(filter);
      } else if (!isa<Instruction>(op1) && op2 && isa<Instruction>(op2) ) {
        code->push_back({.opcode="BPF_ALU | " + bpf_opcodes[arith->GetOpcode()] + " | " + GetAddressMode(op1),
                  .k = GenK_Field(op1, arg_index),
                  .type="BPF_STMT"});
      } else if (isa<Instruction>(op1) && op2 && !isa<Instruction>(op2) ) {
        code->push_back({.opcode="BPF_ALU | " + bpf_opcodes[arith->GetOpcode()] + " | " + GetAddressMode(op2),
                  .k = GenK_Field(op2, arg_index),
                  .type="BPF_STMT"});
      } else if (!op2) {
        if (!isa<Instruction>(op1)) {
          LoadArgumentOrInt(op1, arg_index, code);
        }
        if (arith->GetOpcode() != OpNode::UMINUS) {
          code->push_back({.opcode="BPF_ALU | " + bpf_opcodes[arith->GetOpcode()],
                    .type="BPF_STMT"});
        } else {
          code->push_back({.opcode="BPF_TAX",
                    .type="BPF_STMT"});
          code->push_back({.opcode="BPF_LD | BPF_W | BPF_K",
                    .k = "0",
                    .type="BPF_STMT"});
          code->push_back({.opcode="BPF_ALU | BPF_SUB | BPF_X",
                    .type="BPF_STMT"});
        }
      } else if (!isa<Instruction>(op1) && !isa<Instruction>(op2)) {
        LoadArgumentOrInt(op1, arg_index, code);
       
        code->push_back({.opcode="BPF_ALU | " + bpf_opcodes[arith->GetOpcode()] + " | " + GetAddressMode(op2),
                .k = GenK_Field(op2, arg_index),
                .type="BPF_STMT"});
        
      } else {
        internalErr("Unknown case: " + arith->String());
      }
      A_val = arith;
    } else if(isa<BranchInst>(inst)) {
      BranchInst * branch = (BranchInst*)(inst);
      IRValue * op1 = branch->GetOperand(0);
      IRValue * op2 = branch->GetOperand(1);
      auto jmp_type = branch->GetJmpType();
      string true_target = branch->GetTrueTarget();
      string false_target = branch->GetFalseTarget();
      if (branch->IsUncondBranch()) {
        filter = {.opcode="BPF_JMP | BPF_JA", 
                  .k = branch->GetTrueTarget(),
                  .jt="0",
                  .jf="0",
                  .type="BPF_JUMP"};
      } else if (isa<Instruction>(op1) && isa<Instruction>(op2)) {
        assert(op1 == M0_val || op2 == M0_val);
        filter = {.opcode=  "BPF_LDX | BPF_MEM | BPF_W", .k="0", .type="BPF_STMT"};
        M0_val = nullptr;
        code->push_back(filter);
        filter = {.opcode="BPF_JMP | BPF_X | " + bpf_opcodes[jmp_type], 
                  .k = "0",
                  .jt= NeedSwapTarget(jmp_type) ? false_target : true_target ,
                  .jf= NeedSwapTarget(jmp_type) ? true_target : false_target ,
                  .type="BPF_JUMP"};
        code->push_back(filter);
      } else if (!isa<Instruction>(op1)&& isa<Instruction>(op2) ) {
        code->push_back({.opcode="BPF_JMP | " + bpf_opcodes[jmp_type] + " | " + GetAddressMode(op1),
                  .k = GenK_Field(op1, arg_index),
                  .jt=NeedSwapTarget(jmp_type) ? false_target : true_target ,
                  .jf=NeedSwapTarget(jmp_type) ? true_target : false_target ,
                  .type="BPF_JUMP"});
      } else if (isa<Instruction>(op1) && !isa<Instruction>(op2)) {
        code->push_back({.opcode="BPF_JMP | " + bpf_opcodes[jmp_type] + " | " + GetAddressMode(op2),
                  .k = GenK_Field(op2, arg_index),
                  .jt=NeedSwapTarget(jmp_type) ? false_target : true_target ,
                  .jf=NeedSwapTarget(jmp_type) ? true_target : false_target,
                  .type="BPF_JUMP"});
      } else if (!isa<Instruction>(op1) && !isa<Instruction>(op2)) {
        LoadArgumentOrInt(op1, arg_index, code);
        code->push_back({.opcode="BPF_JMP | " + bpf_opcodes[jmp_type] + " | " + GetAddressMode(op2),
                  .k = GenK_Field(op2, arg_index),
                  .jt=NeedSwapTarget(jmp_type) ? false_target : true_target,
                  .jf=NeedSwapTarget(jmp_type) ? true_target : false_target,
                  .type="BPF_JUMP"});
      } else {
        internalErr("Unknown case: " + branch->String());
      }
      A_val = nullptr;
    } else if (isa<ReturnInst>(inst)) {
      ReturnInst * ret = (ReturnInst*)inst;
      Immediate<int> * imm = (Immediate<int> * )ret->GetOperand(0);
      int retval = imm->GetValue();
      switch (retval)
      {
      case PermNode::ALLOW:
        code->push_back({.opcode="BPF_RET | BPF_K",
                        .k = "SECCOMP_RET_ALLOW",
                        .type="BPF_STMT"});
        break;
      case PermNode::DENY:
        code->push_back({.opcode="BPF_RET | BPF_K",
                        .k = DENY_ACTION ,
                        .type="BPF_STMT"});
        break;
      case RET_TO_PTRACE:
      {
        int retdata = bpfret_to_ret_data[inst];
        string k_str =  "SECCOMP_RET_TRACE | (SECCOMP_RET_DATA & "  + to_string(retdata) + ")";
        code->push_back({.opcode="BPF_RET | BPF_K",
                        .k = k_str,
                        .type="BPF_STMT"});
        break;
      }
      default:
        internalErr("Unknown case: " + ret->String());
        break;
      }
    } else if (isa<LabelInst>(inst)) {
      code->push_back({.label=inst->String()});
    }
  }
  return code;
}

string StringfyBPF_Filter(const BPF_Filter & filter) {
  string bpf_code;
  if (!filter.label.empty()) {
    return filter.label;
  }
  bpf_code = filter.type + "(" + filter.opcode;

  bpf_code += ", " + (filter.k.empty() ? "0" : filter.k);

  if (!filter.jt.empty()) {
    bpf_code += ", " + filter.jt;
  }
  if (!filter.jf.empty()) {
    bpf_code += ", " + filter.jf;
  }
  bpf_code += ")";
  return bpf_code;
}

void BPFTransformLabels(list<BPF_Filter> & filters) {
  unordered_map<string, BPF_Filter*>  label_2_filter;
  auto it = filters.begin();
  while (it != filters.end()) {
    BPF_Filter * f = &(*it);
    auto next = ++it;
    if (!f->label.empty()) {
      label_2_filter[f->label] = &(*next);
    }
  }
  // transform string target to address offset
  // first allocating ip
  int ip = 0;
  for (auto & f : filters) {
    f.pc = ip;
    if (f.label.empty()) {
      ip++;
    }
  }
  it = filters.begin();
  while (it != filters.end()) {
    auto cur = it++;
    BPF_Filter & f = *cur;
    if (!f.label.empty()) {
      filters.erase(cur);
      continue;
    }
    if (f.type == "BPF_JUMP") {
      int offset;
      if (f.opcode.find("JA") != string::npos) {
        BPF_Filter * target = label_2_filter[f.k];
        if (target) {
          offset = target->pc - f.pc -1;
          f.k = to_string(offset);
        }
        continue;
      }
      BPF_Filter * true_target = label_2_filter[f.jt];
      
      if (true_target) {
        offset = true_target->pc - f.pc - 1;
        f.jt = to_string(offset);
      }
      BPF_Filter * false_target = label_2_filter[f.jf];
      if (false_target) {
        offset = false_target->pc - f.pc - 1;
        f.jf = to_string(offset);
      }
    }
  }
}
