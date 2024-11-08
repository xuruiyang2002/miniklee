#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stack>
#include <iostream>
#include "ExecutionState.h"

class Executor {
public:
    std::stack<ExecutionState> stateStack;

    void runFunctionAsMain(llvm::Function* function);

    // Executor(llvm::LLVMContext& context) : context_(context) {}

    Executor() {}

private:
    void stepInstruction(ExecutionState& state);

    void executeInstruction(ExecutionState& state, llvm::Instruction* inst);

    void updateStates(ExecutionState *current);

    void transferToBasicBlock(llvm::BasicBlock *dst, ExecutionState &state);
};

#endif // EXECUTOR_H