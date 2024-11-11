#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stack>
#include <iostream>
#include "ExecutionState.h"

using namespace llvm;

class Executor {
public:
    std::unique_ptr<llvm::Module> module;

    std::stack<ExecutionState> stateStack;

    void runFunctionAsMain(llvm::Function* function);

    // Constructor that accepts an llvm::Module pointer
    explicit Executor(std::unique_ptr<llvm::Module> module) 
        : module(std::move(module)) {}

private:

    void stepInstruction(ExecutionState& state);

    void executeInstruction(ExecutionState& state, llvm::Instruction* inst);

    void updateStates(ExecutionState *current);

    void transferToBasicBlock(llvm::BasicBlock *dst, ExecutionState &state);

    void executeAlloc(ExecutionState& state, unsigned size, llvm::Instruction* inst);

    ref<Expr> getValue(llvm::Instruction* i, ExecutionState& state);

    void executeMemoryOperation(ExecutionState& state, bool isWrite, llvm::Instruction *address, ref<Expr> value, llvm::Instruction* i);

    int32_t getInt32Helper(ExecutionState& state, Value* value /* ConstantInt* or Instruction* */);
        
    void executeMakeSymbolic(ExecutionState& state, Instruction *sym, std::string name);
};

#endif // EXECUTOR_H