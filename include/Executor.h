#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stack>
#include <iostream>
#include "ExecutionState.h"
#include "Solver.h"

using namespace llvm;

class Executor {
public:
    std::unique_ptr<llvm::Module> module;
    
    std::unique_ptr<Solver> solver;

    std::stack<ExecutionState> stateStack;

    typedef std::pair<ExecutionState*,ExecutionState*> StatePair;

    void runFunctionAsMain(llvm::Function* function);

    // Constructor that accepts an llvm::Module pointer
    explicit Executor(std::unique_ptr<llvm::Module> module);

    // Used to track states that have been added during the current
    // instructions step. 
    std::vector<ExecutionState *> addedStates;
    // Used to track states that have been removed during the current
    // instructions step. 
    std::vector<ExecutionState *> removedStates;

private:

    void stepInstruction(ExecutionState& state);

    void executeInstruction(ExecutionState& state, llvm::Instruction* inst);

    void updateStates(ExecutionState *current);

    void transferToBasicBlock(llvm::BasicBlock *dst, ExecutionState &state);

    void executeAlloc(ExecutionState& state, unsigned size, llvm::Instruction* inst);

    ref<Expr> getInstructionValue(ExecutionState& state, llvm::Instruction* i);

    void executeMemoryOperation(ExecutionState& state, bool isWrite, llvm::Instruction *address, ref<Expr> value, llvm::Instruction* i);

    ref<Expr> getValue(ExecutionState& state, Value* value /* ConstantInt* or Instruction* */);
        
    void executeMakeSymbolic(ExecutionState& state, Instruction *sym, std::string name);

    StatePair fork(ExecutionState &current, ref<Expr> condition);

    /// Add the given (boolean) condition as a constraint on state. This
    /// function is a wrapper around the state's addConstraint function.
    void addConstraint(ExecutionState &state, ref<Expr> condition);

};

#endif // EXECUTOR_H