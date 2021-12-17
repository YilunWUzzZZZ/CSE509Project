#ifndef IR_LIFTER_H
#define IR_LIFTER_H
#include <string>
#include <stack>
#include <vector>
#include <list>
#include <unordered_map>
#include <set>

using namespace std;
class Instruction;
class BasicBlock;

#define PTRACE_GOTO "goto"
#define PTRACE_IF "if"
#define PTRACE_ALLOW "ALLOW"
#define PTRACE_DENY "DENY"
#define PTRACE_ASSIGN "="
#define PTRACE_STMT "stmt"
#define PTRACE_LABEL "label"

struct PtraceCode {
  string type;
  string expr;
  string dst;
  string true_target;
  string false_target;
  string label;
};


class IRLifter {
public:
  IRLifter() {};
  ~IRLifter();

 void LiftBasicBlock(BasicBlock * bb);
 void LiftInst(Instruction * inst,  list<PtraceCode*>* code, const string & label);
 void GenCode();
 void GenBasicBlockCode(list<PtraceCode*>* bb, const string & label, int indent);
 void Init(set<string> * interface_points,set<string> * string_args, unordered_map<string, string> * mem_args_size,
           unordered_map<string, string> *arg_types, unordered_map<string, int> *arg_index) {
        interface_points_ = interface_points;
        string_args_ = string_args;
        mem_args_size_ = mem_args_size;
        arg_types_ = arg_types;
        arg_index_ = arg_index;
      }
 vector<string> & Code() { return C_code_; }
 static int ret_data;
private:
  vector<string> C_code_;
  unordered_map<string, list<PtraceCode*>*> code_blocks_;
  unordered_map<string, int> block_ref_cnt_;
  unordered_map<string, bool> used_; 
  vector<string> block_order_;
public:
  set<string> * interface_points_;
  set<string> * string_args_;
  unordered_map<string, string> * mem_args_size_;
  unordered_map<string, string> *arg_types_;
  unordered_map<string, int> *arg_index_;

private:
  void __LiftInst(Instruction * inst, list<PtraceCode*>* code);
};

void PrintCCode(ostream & os, const vector<string> & code);
#endif