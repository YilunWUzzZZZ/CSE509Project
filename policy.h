#ifndef POLICY_H
#define POLICY_Y

#include "ast.h"
#include "code_gen.h"
#include <unordered_map>
class BasicBlock;
class IRLifter;

class Policy {
 public:
  enum PolicyType {
      SYSCALL_CHECK,
      SYSCALL_EXT,
      DEFAULT_DENY,
      DEFAULT_ALLOW,
  };

  Policy(PolicyType type): type_(type) {}
  ~Policy() {};
  PolicyType Type() { return type_; }
  virtual void Print(ostream & os, int indent=0);
 private:
  PolicyType type_;
};


class PolicyManager;
class SyscallCheck: public Policy {
 private:
  CompoundStmtNode* stmts_;
  string  syscall_;
  vector<string> *args_;
  unordered_map<string, int> arg_index_;
  list<Instruction*> IR_;
  vector<BasicBlock*> * CFG_;
  list<BPF_Filter> *bpf_filters_;
  IRLifter *lifter_;
  PtraceBasicBlock ptrace_bbs_;
  PtraceBasicBlock ptrace_only_bbs_;
 public:
  friend class PolicyManager;
  SyscallCheck(CompoundStmtNode * stmts, const string & syscall, vector<string> * args = nullptr): 
    Policy(PolicyType::SYSCALL_CHECK), stmts_(stmts), syscall_(syscall), args_(args) { 
      for (size_t i=0; i<args_->size(); i++) {
        arg_index_[(*args_)[i]] = i; 
      }
    }
  ~SyscallCheck() {}
  
  inline bool HasArgument(const string arg) {
      if (!args_) return false;
      for (auto a : *args_) {
          if (a == arg) {
              return true;
          }
      }
      return false;
  }

  void SetArgs(vector<string> * args) { args_ = args; }
  void SetStmts(CompoundStmtNode* stmts) { stmts_ = stmts; }
  list<Instruction*> & IR() {return IR_;}
  void IRGen(CodeGenMgr & mgr);
  void CodeGen(CodeGenMgr & mgr);
  void PrintIR();
  void Print(ostream & os, int indent=0);
};

class SyscallExtension: public Policy {
 private:
  string  extender_;
  string  syscall_;
 public:
  SyscallExtension(const string & syscall, const string & extender): 
    Policy(PolicyType::SYSCALL_EXT), syscall_(syscall), extender_(extender) {}
  ~SyscallExtension() {}
  void Print(ostream & os, int indent=0);
};

class PolicyManager {
 private:
  vector<Policy*> policys_;
  Policy*  cur_policy_;
  vector<BPF_Filter> bpf_filters_;

  string bpf_output_file_;
  string ptrace_output_file_;

 public:
  PolicyManager() {}
  ~PolicyManager() {}
  void AddPolicy(Policy * p) { policys_.push_back(p); cur_policy_= p; }
  inline const vector<Policy*> & GetAllPolicy() const { return policys_; }
  inline Policy *CurPolicy()  { return  cur_policy_; }
  void Print(ostream & os, int indent=0);
  void PtraceCodeGen(CodeGenMgr & mgr);
  void BPFCodeGen(CodeGenMgr & mgr);
  void CodeGen(CodeGenMgr & mgr);  
};

#endif