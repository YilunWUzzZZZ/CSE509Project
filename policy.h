#ifndef POLICY_H
#define POLICY_Y

#include "ast.h"
class BasicBlock;


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


class SyscallCheck: public Policy {
 private:
  CompoundStmtNode* stmts_;
  string  syscall_;
  vector<string> *args_;
  list<Instruction*> IR_;
  vector<BasicBlock*> * CFG_;
 public:

  SyscallCheck(CompoundStmtNode * stmts, const string & syscall, vector<string> * args = nullptr): 
    Policy(PolicyType::SYSCALL_CHECK), stmts_(stmts), syscall_(syscall), args_(args) { }
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
 public:
  PolicyManager() {}
  ~PolicyManager() {}
  void AddPolicy(Policy * p) { policys_.push_back(p); cur_policy_= p; }
  inline const vector<Policy*> & GetAllPolicy() const { return policys_; }
  inline Policy *CurPolicy()  { return  cur_policy_; }
  void Print(ostream & os, int indent=0);
};

#endif