#include "policy.h"
#include "ast.h"
#include "IR.h"
#include "IR_gen.h"
#include "code_gen.h"
#include "IR_lifter.h"
#include "syscalls.h"
#include <fstream>
#include "code_template.h"
// PolicyManager manager;

void Policy::Print(ostream & os, int indent) {
  if (type_ == PolicyType::DEFAULT_ALLOW) {
    os << "default: allow\n";
  } else if (type_ == PolicyType::DEFAULT_DENY) {
    os << "default: deny\n";
  }

}

void SyscallCheck::Print(ostream & os, int indent) {
  os << "def " << syscall_ << "(";
  if (args_) {
    for (size_t i=0 ; i<args_->size(); i++) {
      os << (*args_)[i];
      if (i < args_->size() - 1) {
        os << ", ";
      }
    }
  }
  os << ") :\n";
  if (stmts_)
    stmts_->Print(os, indent + 2);
}

void SyscallExtension::Print(ostream & os, int indent) {
  os << syscall_ << " : " << extender_ << "\n";
}

void PolicyManager::Print(ostream & os, int indent) {
  for (auto p: policys_) {
    p->Print(os, indent);
    os << "\n";
  }
}

void SyscallCheck::IRGen(CodeGenMgr & mgr) {
  stmts_->IRGen(IR_, mgr);
  MergeLabels(IR_);
  EliminateDeadCode(IR_);
  CFG_ = BuildCFG(IR_, mgr, syscall_);
  // PrintCFG(cout, CFG_);
}
void SyscallCheck::PrintIR() {
  cerr << syscall_ << ":\n";
  for (auto inst: IR_) {
    cerr << inst->String() << "\n"; 
  }
}

void SyscallCheck::CodeGen(CodeGenMgr & mgr) {
  list<Instruction*> BPF_IR;
  IRLifter * lifter = new IRLifter();
  PtraceBasicBlock ptrace_bbs_;
  PtraceBasicBlock ptrace_only_bbs_;

  ColorCode(*CFG_, ptrace_bbs_, ptrace_only_bbs_);
  for (auto p : ptrace_bbs_) {
    cout << p.first->GetLabel() << " ";
  }
  cout << "\n";
  for (auto p : ptrace_only_bbs_) {
    cout << p.first->GetLabel() << " ";
  }
  GenBPFIR(*CFG_, ptrace_only_bbs_, BPF_IR, *lifter);
  GenPtraceCode(*CFG_, ptrace_bbs_, *lifter);
  list<BPF_Filter> *filters = BPFCodeGen(BPF_IR, &arg_index_, *lifter);
  bpf_filters_ = filters;
  lifter_ = lifter;
  // cerr << "\n" << syscall_ << " BPF code: \n";
  // for (auto & f: *filters) {
  //   cerr << StringfyBPF_Filter(f) << "\n"; 
  // }

  // cerr << "\n" << syscall_ << " BPF IR: \n";
  // for (auto inst: BPF_IR) {
  //   cerr << inst->String() << "\n"; 
  // }

  // cerr << "\n" << syscall_ << " C code: \n";
  // PrintCCode(cout, lifter->Code());

}


void PolicyManager::PtraceCodeGen(CodeGenMgr & mgr) {
  vector<string> ptrace_labels;
  vector<SyscallCheck*> checks;
  string ptrace_code;
  string jmp_table = "void * jmp_table[] = {";
  for (auto p : policys_) {
    if (p->Type() == Policy::SYSCALL_CHECK) {
      SyscallCheck * check = (SyscallCheck*)p;
      checks.push_back(check);
      for (auto & pair : check->ptrace_bbs_) {
        ptrace_labels.push_back(pair.first->GetLabel());
      }
    } 
  }
  for (int i=0; i<ptrace_labels.size(); i++) {
    if (i % 15 == 0) {
      jmp_table += "\n";
    }
    jmp_table += "&&" + ptrace_labels[i] + ",";
  }
  jmp_table += "};\n";
  ptrace_code = ptrace_template_p1 + jmp_table + ptrace_template_p1;
  string indent = "            ";
  for (auto check : checks) {
    
  }

  
}

void PolicyManager::CodeGen(CodeGenMgr & mgr) {
  vector<SyscallCheck*> checks;
  bool default_deny = true;

  for (auto p : policys_) {
    if (p->Type() == Policy::SYSCALL_CHECK) {
      SyscallCheck * check = (SyscallCheck*)p;
      check->CodeGen(mgr);
      checks.push_back(check);
    } else if (p->Type() == Policy::DEFAULT_ALLOW) {
      default_deny = false;
    } 
  }

  string default_label = default_deny ?  "__deny" : "__allow";  

  list<BPF_Filter> final_filters;
  // first insert architecture checking code
  final_filters.push_back({.opcode = "BPF_LD | BPF_W | BPF_ABS", .k="(offsetof(struct seccomp_data, arch))", .type = "BPF_STMT"});
  final_filters.push_back({.opcode = "BPF_JMP | BPF_JEQ | BPF_K", .k="t_arch", .jt="0", .jf="__deny",.type = "BPF_JUMP"});
  // check syscall number for x86_64
  final_filters.push_back({.opcode = "BPF_LD | BPF_W | BPF_ABS", .k="(offsetof(struct seccomp_data, nr))", .type = "BPF_STMT"});
  final_filters.push_back({.opcode = "BPF_JMP | BPF_JLT | BPF_K", .k="upper_nr_limit", .jt="0", .jf="__deny",.type = "BPF_JUMP"});
  for (size_t i=0; i<checks.size(); i++) {
    SyscallCheck * c = checks[i];
    SyscallCheck * next = (i + 1 == checks.size())?nullptr:checks[i+1];
    string next_label = next?next->syscall_:default_label;
    // compare syscall numbers
    final_filters.push_back({.label = c->syscall_});
    final_filters.push_back({.opcode = "BPF_JMP | BPF_JEQ | BPF_K", .k=to_string(SyscallNameToNr(c->syscall_)), .jt="0", .jf=next_label,.type = "BPF_JUMP"});
    for (auto & f : *(c->bpf_filters_)) {
      final_filters.push_back(f);
    }
  }
  // add default label
  final_filters.push_back({.label = default_label});
  if (!default_deny) {
    // default allow
    final_filters.push_back({.opcode="BPF_RET | BPF_K",
                        .k = "SECCOMP_RET_ALLOW" ,
                        .type="BPF_STMT"});
   // followed by the deny label
    final_filters.push_back({.label = "__deny"});
  }

  final_filters.push_back({.opcode="BPF_RET | BPF_K",
                        .k = "SECCOMP_RET_KILL_PROCESS" ,
                        .type="BPF_STMT"});
  for (auto & f: final_filters) {
    cerr << StringfyBPF_Filter(f) << "\n"; 
  }
  BPFTransformLabels(final_filters);
  // for (auto & f: final_filters) {
  //   cerr << StringfyBPF_Filter(f) << "\n"; 
  // }
}  