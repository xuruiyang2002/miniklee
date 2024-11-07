#pragma once
// #include <llvm/IR/Function.h>
// #include <llvm/IR/BasicBlock.h>
// #include <llvm/IR/Instruction.h>
// #include <llvm/IR/Instructions.h>
// #include <llvm/IR/Module.h>
// #include <llvm/Support/raw_ostream.h>
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

    void handleBinaryOperation(llvm::BinaryOperator& binOp, ExecutionState& state);
};
