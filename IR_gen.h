#ifndef IR_GEN_H
#define IR_GEN_H

#include <string>
#include <vector>
#include <stack>
#include <list>
#include <unordered_map>

using namespace std;

class Instruction;
class BasicBlock;

class CodeGenMgr {
 public:
  CodeGenMgr(): label_cnt_(0), ip_cnt_(0) {}
  ~CodeGenMgr() {}
  string NewLabel() { return "L" + to_string(++label_cnt_);}
  int NewIP() { return ip_cnt_++; }
  void SaveBranchTarget() { saved_branch_target_.push({true_target, false_target});}
  void RestoreBranchTarget() { 
    vector<string> tars = saved_branch_target_.top();
    saved_branch_target_.pop();
    true_target = tars[0]; false_target = tars[1];
  }
  BasicBlock * NewBasicBlock(const string & label);
  BasicBlock * GetBasicBlock(const string & label) { return label_BBs_[label]; }
 
 private:
  int label_cnt_;
  int ip_cnt_;
  int ptrace_label_id_;
  stack<vector<string>> saved_branch_target_;
  unordered_map<string, BasicBlock*> label_BBs_;
  
 public:
  string true_target;
  string false_target;

};

void MergeLabels(list<Instruction*>&code);
void EliminateDeadCode(list<Instruction*>&code);
vector<BasicBlock*> * BuildCFG(list<Instruction*>&code, CodeGenMgr & mgr, const string & syscall_name);
void PrintCFG(ostream & os, vector<BasicBlock*> * BBs);
bool CheckVariableUse(list<Instruction*>&code, vector<string> &args);
#endif