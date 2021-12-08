#ifndef IR_LIFTER_H
#define IR_LIFTER_H
#include <string>
#include <stack>
#include <vector>
#include <list>
#include <unordered_map>

using namespace std;
class Instruction;
class BasicBlock;

class IRLifter {
public:
  IRLifter(): inst_label_cnt_(0) {};
  ~IRLifter();

 void LiftBasicBlock(BasicBlock * bb);
 void LiftInst(Instruction * inst, const string & label);
 void AddMapping(const string & s) {
   label_to_ret_data_[s] = ret_data++;
 }
 int GetMapping(const string & s) {
   return label_to_ret_data_[s];
 }
 string GenInstLabel(Instruction * i) { 
   string l = "CL" + to_string(++inst_label_cnt_); 
   inst_label_[i] = l; 
   return l;
 }
 string GetInstLabel(Instruction * i) {
   return inst_label_[i];
 }
 vector<string> & Code() { return C_code_; }
 static int ret_data;
private:
  vector<string> C_code_;
  unordered_map<string, unsigned short> label_to_ret_data_;
  int inst_label_cnt_;
  unordered_map<Instruction*, string> inst_label_;
private:
  void __LiftInst(Instruction * inst);
};

void PrintCCode(ostream & os, const vector<string> & code);
#endif