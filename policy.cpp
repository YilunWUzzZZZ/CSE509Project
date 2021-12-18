#include "policy.h"
#include "ast.h"
#include "IR.h"
#include "IR_gen.h"
#include "code_gen.h"
#include "IR_lifter.h"
#include "syscalls.h"
#include <fstream>
#include <iostream>
#include "code_template.h"
// PolicyManager manager;

#ifdef DEBUG
bool DEBUG_ON=true;
#else
bool DEBUG_ON=false;
#endif

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

void SyscallCheck::CodeGen() {
  list<Instruction*> BPF_IR;
  IRLifter * lifter = new IRLifter();


  ColorCode(*CFG_, ptrace_bbs_, ptrace_only_bbs_);
  if (DEBUG_ON) {
    for (auto p : ptrace_bbs_) {
      cout << p.first->GetLabel() << " ";
    }
    cout << "\n";
    for (auto p : ptrace_only_bbs_) {
      cout << p.first->GetLabel() << " ";
    }
    cout << "\n";
  }
  GetArgumentsInfo(SyscallNameToNr(syscall_), *args_, ptrace_bbs_, lifter);
  GenBPFIR(*CFG_, ptrace_only_bbs_, BPF_IR, *lifter);
  lifter->interface_points_ = new set<string>();
  set<string> local_labels;
  for (auto bb: ptrace_bbs_) {
    local_labels.insert(bb.first->GetLabel());
  }
  for (auto pt: ret_data_to_label) {
    if (local_labels.count(pt)) {
      lifter->interface_points_->insert(pt);
    }
  }
  
  GenPtraceCode(*CFG_, ptrace_bbs_, *lifter);
  lifter->GenCode();

  // PrintCCode(cout, lifter->Code());
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

void PolicyManager::IRGen(CodeGenMgr & mgr) {
  for (auto p : policys_) {
    if (p->Type() == Policy::SYSCALL_CHECK) {
      SyscallCheck* ck = (SyscallCheck*)p;
      ck->IRGen(mgr);
      if (DEBUG_ON) {
        cerr << "\n";
        ck->PrintIR();
      }
    }
  }
}

void PolicyManager::PtraceCodeGen() {
  vector<SyscallCheck*> checks;
  string ptrace_code;
  string jmp_table = "  void * jmp_table[] = {";
  string start_table = "  void * start_table[] = {";
  bool has_ptrace = false;
  unordered_map<string, string> label_2_check;
  for (auto p : policys_) {
    if (p->Type() == Policy::SYSCALL_CHECK) {
      SyscallCheck * check = (SyscallCheck*)p;
      checks.push_back(check);
      if (check->ptrace_bbs_.size()) {
        has_ptrace = true;
      }
      for (auto pbb : check->ptrace_bbs_) {
        label_2_check[pbb.first->GetLabel()] = check->syscall_;
      }
    } 
  }
  if (!has_ptrace) {
    return;
  }
  for (int i=0; i<ret_data_to_label.size(); i++) {
    if (i && i % 15 == 0) {
      jmp_table += "\n";
      start_table += "\n";
    }
    string label = ret_data_to_label[i];
    jmp_table += "&&" + label + ",";
    start_table += "&&" + label_2_check[label] + "_start, ";
  }
  jmp_table += "};\n";
  start_table += "};\n";
  ptrace_code = ptrace_template_p1 + jmp_table + start_table + ptrace_template_p2;
  string indent = "            ";
  for (auto check : checks) {
    auto & code = check->lifter_->Code();
    ptrace_code += indent + "{\n";
    vector<string> prologue;
    GenPtracePrologue(SyscallNameToNr(check->syscall_), *(check->args_), check->ptrace_bbs_, check->lifter_, prologue);
    for (auto & line: prologue) {
      ptrace_code += indent + line + "\n";
    }
    ptrace_code += indent + "goto *jmp_table[seccomp_ret_data];\n";
    for (auto & line : code) {
      ptrace_code += "  " + indent + line + "\n";
    }
  
    ptrace_code += indent  + "}\n\n";
  }
  ptrace_code += ptrace_template_p3;
  ofstream ptrace_file(ptrace_output_file_);
  if (ptrace_file.is_open()) {
    ptrace_file << ptrace_code;
    ptrace_file.close();
  } else {
    fprintf(stderr, "Can't open output file %s\n", ptrace_output_file_.c_str());
    exit(EXIT_FAILURE);
  }
}

void PolicyManager::BPFCodeGen(bool default_deny) {
  vector<SyscallCheck*> checks;
  for (auto p : policys_) {
    if (p->Type() == Policy::SYSCALL_CHECK) {
      SyscallCheck * check = (SyscallCheck*)p;
      checks.push_back(check);
    } 
  }
  if (!checks.size()) {
    return;
  }
  string default_label = default_deny ?  "__deny" : "__allow";  
  list<BPF_Filter> final_filters;
// first insert architecture checking code
  final_filters.push_back({.opcode = "BPF_LD | BPF_W | BPF_ABS", .k="(offsetof(struct seccomp_data, arch))", .type = "BPF_STMT"});
  final_filters.push_back({.opcode = "BPF_JMP | BPF_JEQ | BPF_K", .k="t_arch", .jt="0", .jf="__deny",.type = "BPF_JUMP"});
// check syscall number for x86_64
  final_filters.push_back({.opcode = "BPF_LD | BPF_W | BPF_ABS", .k="(offsetof(struct seccomp_data, nr))", .type = "BPF_STMT"});
  final_filters.push_back({.opcode = "BPF_JMP | BPF_JGT | BPF_K", .k="upper_nr_limit", .jt="0", .jf=checks[0]->syscall_, .type = "BPF_JUMP"});
// x86_32 ABI, unset the bit
  final_filters.push_back({.opcode = "BPF_ALU | BPF_AND | BPF_K", .k="~X32_SYSCALL_BIT", .type = "BPF_STMT"});
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
  if (DEBUG_ON) {
    for (auto & f: final_filters) {
      cerr << StringfyBPF_Filter(f) << "\n"; 
    }
  }
  BPFTransformLabels(final_filters);
  // for (auto & f: final_filters) {
  //   cerr << StringfyBPF_Filter(f) << "\n"; 
  // }
  string indent = "        ";
  string output = seccomp_template_p1;
  for (auto & f: final_filters) {
    output += indent + StringfyBPF_Filter(f)+ ",\n"; 
  }
  output += seccomp_template_p2;
  ofstream bpf_file(bpf_output_file_);
  if (bpf_file.is_open()) {
    bpf_file << output;
    bpf_file.close(); 
  } else {
    fprintf(stderr, "Can't open output file %s\n", bpf_output_file_.c_str());
    exit(EXIT_FAILURE);
  }
}


void PolicyManager::CodeGen() {
  bool default_deny = true;
  for (auto p : policys_) {
    if (p->Type() == Policy::SYSCALL_CHECK) {
      SyscallCheck * check = (SyscallCheck*)p;
      check->CodeGen();
    } else if (p->Type() == Policy::DEFAULT_ALLOW) {
      default_deny = false;
    } 
  }
  BPFCodeGen(default_deny);
  PtraceCodeGen();
}  