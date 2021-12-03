#include "policy.h"
#include "ast.h"
#include "IR.h"
#include "IR_gen.h"
#include "code_gen.h"
#include "IR_lifter.h"
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
  PtraceBasicBlock ptrace_bbs;
  PtraceBasicBlock ptrace_only_bbs;

  ColorCode(*CFG_, ptrace_bbs, ptrace_only_bbs);
  for (auto p : ptrace_bbs) {
    cout << p.first->GetLabel() << " ";
  }
  cout << "\n";
  for (auto p : ptrace_only_bbs) {
    cout << p.first->GetLabel() << " ";
  }
  GenBPFIR(*CFG_, ptrace_only_bbs, BPF_IR, *lifter);
  GenPtraceCode(*CFG_, ptrace_bbs, *lifter);
  cerr << "\n" << syscall_ << " BPF IR: \n";
  for (auto inst: BPF_IR) {
    cerr << inst->String() << "\n"; 
  }

  cerr << "\n" << syscall_ << " C code: \n";
  PrintCCode(cout, lifter->Code());

}