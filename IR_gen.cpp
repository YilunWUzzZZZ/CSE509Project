#include "IR_gen.h"
#include "IR.h"
#include <unordered_map>
#include <queue>
#include <set>

BasicBlock * CodeGenMgr::NewBasicBlock(const string & label) {
  label_BBs_[label] = new BasicBlock(label);
  return label_BBs_[label];
}

void MergeLabels(list<Instruction*>&code) {
  list<Instruction*>::iterator it = code.begin();
  unordered_map<string, vector<Instruction*>> label_insts;
  bool prev_is_label = false;
  string prev_label;
  unordered_map<string, string> old_label_2_new;

  for (auto inst : code) {
    if (isa<BranchInst>(inst)) {
      BranchInst * b = (BranchInst*)inst;
      label_insts[b->GetTrueTarget()].push_back(b);
      if ( !b->IsUncondBranch() ) {
        label_insts[b->GetFalseTarget()].push_back(b);
      } 
    }
  }

  while (it != code.end()) {
    auto cur_it = it++;

    if (isa<LabelInst>(*cur_it)) {
      LabelInst * l = (LabelInst*)(*cur_it);
      if (prev_is_label) {
        old_label_2_new[l->String()] = prev_label;
        code.erase(cur_it);
        delete l;
      } else {
        prev_is_label = true;
        prev_label = l->String();
      }
    } else {
      prev_is_label = false;
    }  
  } 

  int i = 0;
  for (auto inst : code) {
    if (!isa<LabelInst>(inst) && !isa<BranchInst>(inst) && !isa<StoreInst>(inst)) {
      i++;
      inst->SetDstReg(to_string(i));
    }
    if (isa<BranchInst>(inst)) {
      BranchInst * b = (BranchInst*)inst;
      string true_label = b->GetTrueTarget();
      string false_label = b->GetFalseTarget();
      if (old_label_2_new.find(true_label) != old_label_2_new.end()) {
        b->SetTrueTarget(old_label_2_new[true_label]);
      }
      if (!b->IsUncondBranch()) {
        if (old_label_2_new.find(false_label) != old_label_2_new.end()) {
          b->SetFalseTarget(old_label_2_new[false_label]);
        }
      } 
    }
  }
  
}

static bool IsDirectJMP(Instruction * inst) {
  if (isa<BranchInst>(inst) && static_cast<BranchInst*>(inst)->IsUncondBranch()) {
    return true;
  }
  return false;
}


static bool EliminateDeadCodeImpl(list<Instruction*>&code) {
  // elimnate unreachable BasicBlocks
  auto it = code.begin();
  unordered_map<string, bool> reachable;
  Instruction * prev = nullptr;
  bool purging = false;
  bool modified = false;
  vector<Instruction*> dels;
  while (it != code.end()) {
    auto cur_it = it++;
    Instruction * inst = *cur_it;
    if (isa<LabelInst>(inst)) {
      purging = false;
      LabelInst * l = (LabelInst*)l;
      if (!reachable[inst->String()]) {
        if (prev && (isa<ReturnInst>(prev) || isa<BranchInst>(prev))) {
          purging = true;
        }
      } 
    } 

    if (purging) {
      code.erase(cur_it);
      modified = true;
      dels.push_back(inst);
    } else if (isa<BranchInst>(inst)) {
      BranchInst * b = (BranchInst*)inst;
      reachable[b->GetTrueTarget()] = true;
      reachable[b->GetFalseTarget()] = true;
    }
    prev = inst;
  }
  for (auto i : dels) {
    delete i;
  }
  return modified;
}

void EliminateDeadCode(list<Instruction*>&code) {
  // eliminate dead code after return
  Instruction *prev = nullptr;
  auto it = code.begin();
  while (it != code.end()) {
    auto cur_it = it++;
    Instruction * inst = *cur_it;
    if (prev && ( isa<ReturnInst>(prev) || IsDirectJMP(inst) ) && !isa<LabelInst>(inst)) {
      code.erase(cur_it);
      delete inst;
      continue;
    }
    prev = inst;

  }

  while(EliminateDeadCodeImpl(code)) {
    ; // loop until unchanged
  }

}

// return root BB
vector<BasicBlock*> * BuildCFG(list<Instruction*>&code, CodeGenMgr & mgr, const string & syscall_name) {
  BasicBlock *root = new BasicBlock(syscall_name);
  BasicBlock * cur_BB = root;
  auto it= code.begin();
  unordered_map<string, vector<BasicBlock*>> backward;
  vector<BasicBlock*> * ordered_BBs = new vector<BasicBlock*>();
  ordered_BBs->push_back(root);
  bool prev_branch_or_ret = false;
  
  while (it != code.end()) {
    auto cur_it = it++;
    Instruction * inst = *cur_it;
    if (isa<LabelInst>(inst)) {
      LabelInst * l = (LabelInst*)inst;
      BasicBlock * new_BB = mgr.NewBasicBlock(l->String());
      ordered_BBs->push_back(new_BB);
      if (!prev_branch_or_ret) {
        cur_BB->AddChild(new_BB);
        new_BB->AddParent(cur_BB);
      }
      auto & parents = backward[l->String()];
      for (auto BB: parents) {
        BB->AddChild(new_BB);
        new_BB->AddParent(BB);
      }
      cur_BB = new_BB;
      prev_branch_or_ret = false;
      continue;
    } else if (isa<BranchInst>(inst)) {
      BranchInst * b = (BranchInst*)(inst);
      string true_target =  b->GetTrueTarget();
      string false_target = b->GetFalseTarget();
      backward[true_target].push_back(cur_BB);
    
      if (!b->IsUncondBranch()) {
        backward[false_target].push_back(cur_BB);
      }
      prev_branch_or_ret = true;
    } else if (isa<ReturnInst>(inst)){
      prev_branch_or_ret = true;
    } else {
      prev_branch_or_ret = false;
    }
    inst->SetBasicBlock(cur_BB);
    inst->SetIP(mgr.NewIP());
    cur_BB->AddInst(inst);
  }

  return ordered_BBs;
}

void PrintCFG(ostream & os, vector<BasicBlock*> * BBs) {
  for (auto BB : *BBs) {
    os << BB->GetLabel() << " children: ";
    for (auto c: BB->GetChildren()) {
      os << c->GetLabel() << " ";
    }
    os << "\n";
    for (auto i : BB->Insts()) {
      os << i->String() << "\n";
    }
  }
}

bool CheckVariableUse(list<Instruction*>&code, vector<string> &args) {
  set<string> args_declared;
  for (auto s : args) {
    args_declared.insert(s);
  }
  for (auto inst : code) {
    if (isa<CallInst>(inst)) {
      CallInst * call = (CallInst*)inst;
      auto fargs = call->GetArgs();
      for (auto & arg : *fargs) {
        if (!args_declared.count(arg)) {
          fprintf(stderr, "Undeclared variable %s\n", arg.c_str());
          return false;
        }
      }
    } else if (isa<ArithmeticInst>(inst) || isa<BranchInst>(inst) || isa<StoreInst>(inst)) {
      IRValue * op;
      for (int i=0; i<2; i++) {
        op = inst->GetOperand(i);
        if (op && isa<Memory>(op)) {
          if (!args_declared.count(op->String())) {
            fprintf(stderr, "Undeclared variable %s\n", op->String().c_str());
            return false;
          }
        }
      }
    } 
  }
  return true;
}




